#include <catch2/catch_all.hpp>

#include <boost/uuid.hpp>

#include "boost/uuid/random_generator.hpp"
#include "irods/client_connection.hpp"
#include "irods/connection_pool.hpp"
#include "irods/data_object_modify_info.h"
#include "irods/dstream.hpp"
#include "irods/filesystem.hpp"
#include "irods/getRodsEnv.h"
#include "irods/irods_at_scope_exit.hpp"
#include "irods/irods_client_api_table.hpp"
#include "irods/irods_pack_table.hpp"
#include "irods/rodsClient.h"
#include "irods/rodsErrorTable.h"
#include "irods/touch.h"
#include "irods/transport/default_transport.hpp"

#include "irods/resource_administration.hpp"

#include <array>
#include <climits>
#include <filesystem>

namespace fs = irods::experimental::filesystem;
namespace io = irods::experimental::io;
namespace adm = irods::experimental::administration;

TEST_CASE("data_obj_stat_api")
{    
    load_client_api_plugins();

    rodsEnv env;
    _getRodsEnv(env);

    irods::experimental::client_connection conn;

    const auto sandbox = fs::path{env.rodsHome} / "irods_unit_tests_sandbox";
    const auto path = sandbox / "dstream_data_object.txt";

    fs::client::create_collection(conn, sandbox);

    irods::at_scope_exit cleanup{[&] {
        fs::client::remove_all(conn, sandbox, fs::remove_options::no_trash);
    }};

    // Create a data object in iRODS.
    // This is used in all future sections.
    {
        io::client::default_transport tp{conn};
        io::odstream out{tp, path};
    }

    // Just ensure you can get something in a stat
    SECTION("Stat on good data object") {
        REQUIRE_NOTHROW(fs::client::status(conn, path));
    }

    namespace adm = irods::experimental::administration;

    // Get hostname for unixfilesystem resources
    std::array<char, HOST_NAME_MAX+1> hostname{};
    REQUIRE(gethostname(hostname.data(), hostname.size()) == 0);

    // Sanity check
    // Make sure the hostname buffer is null terminated
    REQUIRE(hostname.back() == '\0');

    auto create_temp_directory{[](std::string_view _dir_name) -> std::filesystem::path {
        const auto temp_path{std::filesystem::temp_directory_path()};
        auto path_to_create{temp_path / _dir_name};
        std::filesystem::create_directory(path_to_create);
        return path_to_create;
    }};

    // Create some temp directories for the resources
    const auto res_a_path{create_temp_directory("cool")};
    const auto res_b_path{create_temp_directory("thing")};

    // Cleanup the temp directories
    irods::at_scope_exit remove_temp_directories{[&](){
        std::filesystem::remove_all(res_b_path);
        std::filesystem::remove_all(res_a_path);
    }};

    // Create resources
    // Should you even test for no throw here?
    const adm::resource_registration_info res_regis_a{.resource_name="cool", .resource_type=adm::resource_type::unixfilesystem, .host_name=hostname.data(), .vault_path=res_a_path.string()};
    REQUIRE_NOTHROW(adm::client::add_resource(conn, res_regis_a));

    const adm::resource_registration_info res_regis_b{.resource_name="thing", .resource_type=adm::resource_type::unixfilesystem, .host_name=hostname.data(), .vault_path=res_b_path.string()};
    REQUIRE_NOTHROW(adm::client::add_resource(conn, res_regis_b));

    const adm::resource_registration_info res_regis_repl{.resource_name="repl", .resource_type=adm::resource_type::replication};
    REQUIRE_NOTHROW(adm::client::add_resource(conn, res_regis_repl));

    const adm::resource_registration_info res_regis_pt{.resource_name="pt", .resource_type=adm::resource_type::passthrough};
    REQUIRE_NOTHROW(adm::client::add_resource(conn, res_regis_pt));

    // Cleanup the resources
    irods::at_scope_exit clean_resources{[&](){
        adm::client::remove_resource(conn, res_regis_pt.resource_name);
        adm::client::remove_resource(conn, res_regis_repl.resource_name);
        adm::client::remove_resource(conn, res_regis_b.resource_name);
        adm::client::remove_resource(conn, res_regis_a.resource_name);
    }};

    // // Close connection and create new one to "commit" previous actions
    conn.disconnect();
    conn.connect();

    // Create the hierarchy
    REQUIRE_NOTHROW(adm::client::add_child_resource(conn, res_regis_pt.resource_name, res_regis_repl.resource_name));
    REQUIRE_NOTHROW(adm::client::add_child_resource(conn, res_regis_repl.resource_name, res_regis_a.resource_name));
    REQUIRE_NOTHROW(adm::client::add_child_resource(conn, res_regis_repl.resource_name, res_regis_b.resource_name));

    // Destruct the hierarchy
    irods::at_scope_exit unlink_resource_hierarchy{[&](){
        adm::client::remove_child_resource(conn, res_regis_repl.resource_name, res_regis_b.resource_name);
        adm::client::remove_child_resource(conn, res_regis_repl.resource_name, res_regis_a.resource_name);
        adm::client::remove_child_resource(conn, res_regis_pt.resource_name, res_regis_repl.resource_name);
    }};

    const auto bleh{sandbox / "cool-cool-epic.txt"};
    {
        io::client::default_transport tp{conn};
        io::odstream out{tp, bleh, io::root_resource_name{res_regis_pt.resource_name}};
    }

    // TOOD: Use bad status? (-1)
}

struct TestFixture {
    TestFixture() : {
        load_client_api_plugins();

        // Get hostname for unixfilesystem resources
        std::array<char, HOST_NAME_MAX+1> hostname{};
        REQUIRE(gethostname(hostname.data(), hostname.size()) == 0);

        // Sanity check
        // Make sure the hostname buffer is null terminated
        REQUIRE(hostname.back() == '\0');

        auto create_temp_directory{[](std::string_view _dir_name) -> std::filesystem::path {
            const auto temp_path{std::filesystem::temp_directory_path()};
            auto path_to_create{temp_path / _dir_name};
            std::filesystem::create_directory(path_to_create);
            return path_to_create;
        }};

        // Create some temp directories for the resources
        const auto res_a_path{create_temp_directory("cool")};
        const auto res_b_path{create_temp_directory("thing")};

        // Create resources
        // Should you even test for no throw here?
        const adm::resource_registration_info res_regis_a{.resource_name="cool", .resource_type=adm::resource_type::unixfilesystem, .host_name=hostname.data(), .vault_path=res_a_path.string()};
        REQUIRE_NOTHROW(adm::client::add_resource(conn, res_regis_a));

        const adm::resource_registration_info res_regis_b{.resource_name="thing", .resource_type=adm::resource_type::unixfilesystem, .host_name=hostname.data(), .vault_path=res_b_path.string()};
        REQUIRE_NOTHROW(adm::client::add_resource(conn, res_regis_b));

        const adm::resource_registration_info res_regis_repl{.resource_name="repl", .resource_type=adm::resource_type::replication};
        REQUIRE_NOTHROW(adm::client::add_resource(conn, res_regis_repl));

        const adm::resource_registration_info res_regis_pt{.resource_name="pt", .resource_type=adm::resource_type::passthrough};
        REQUIRE_NOTHROW(adm::client::add_resource(conn, res_regis_pt));
        
        // Close connection and create new one to "commit" previous actions
        conn.disconnect();
        conn.connect();

        // Create the hierarchy
        REQUIRE_NOTHROW(adm::client::add_child_resource(conn, res_regis_pt.resource_name, res_regis_repl.resource_name));
        REQUIRE_NOTHROW(adm::client::add_child_resource(conn, res_regis_repl.resource_name, res_regis_a.resource_name));
        REQUIRE_NOTHROW(adm::client::add_child_resource(conn, res_regis_repl.resource_name, res_regis_b.resource_name));
    }

    ~TestFixture() {
        // Cleanup all of the files
        fs::client::remove_all(conn, sandbox, fs::remove_options::no_trash);

        // Destruct the hierarchy
        adm::client::remove_child_resource(conn, res_regis_repl.resource_name, res_regis_b.resource_name);
        adm::client::remove_child_resource(conn, res_regis_repl.resource_name, res_regis_a.resource_name);
        adm::client::remove_child_resource(conn, res_regis_pt.resource_name, res_regis_repl.resource_name);

        // Cleanup the resources
        adm::client::remove_resource(conn, res_regis_pt.resource_name);
        adm::client::remove_resource(conn, res_regis_repl.resource_name);
        adm::client::remove_resource(conn, res_regis_b.resource_name);
        adm::client::remove_resource(conn, res_regis_a.resource_name);

        // Cleanup the temp directories
        std::filesystem::remove_all(res_b_path);
        std::filesystem::remove_all(res_a_path);
    }
    
    boost::uuids::uuid test_uuid;
    irods::experimental::client_connection conn;
}

TEST_CASE_METHOD(TestFixture, "Stat on data object with only good replicas") {
    REQUIRE_NOTHROW(fs::client::status(conn, bleh));
}

TEST_CASE_METHOD(TestFixture, "Stat on data object with mixed stale and good replicas") {
    REQUIRE_NOTHROW(fs::client::status(conn, bleh));
}

TEST_CASE_METHOD(TestFixture, "Stat on data object with only stale replicas") {
    REQUIRE_NOTHROW(fs::client::status(conn, bleh));
}