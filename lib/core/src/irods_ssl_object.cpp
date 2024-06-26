// =-=-=-=-=-=-=-
#include "irods/irods_ssl_object.hpp"
#include "irods/irods_network_manager.hpp"

#include "irods/rodsKeyWdDef.h"
#include "irods/rcMisc.h"

extern int ProcessType;

namespace irods {
// =-=-=-=-=-=-=-
// public - ctor
    ssl_object::ssl_object() :
        ssl_ctx_( 0 ),
        ssl_( 0 ),
        key_size_( 0 ),
        salt_size_( 0 ),
        num_hash_rounds_( 0 ) {

    } // ctor

// =-=-=-=-=-=-=-
// public - ctor
    ssl_object::ssl_object(
        const rcComm_t& _comm ) :
        network_object( _comm ),
        ssl_ctx_( _comm.ssl_ctx ),
        ssl_( _comm.ssl ),
        host_( _comm.host ),
        key_size_( _comm.key_size ),
        salt_size_( _comm.salt_size ),
        num_hash_rounds_( _comm.num_hash_rounds ),
        encryption_algorithm_( _comm.encryption_algorithm ) {
    } // ctor

// =-=-=-=-=-=-=-
// public - ctor
    ssl_object::ssl_object(
        const rsComm_t& _comm ) :
        network_object( _comm ),
        ssl_ctx_( _comm.ssl_ctx ),
        ssl_( _comm.ssl ),
        host_( "" ),
        key_size_( _comm.key_size ),
        salt_size_( _comm.salt_size ),
        num_hash_rounds_( _comm.num_hash_rounds ),
        encryption_algorithm_( _comm.encryption_algorithm ) {
    } // ctor

// =-=-=-=-=-=-=-
// public - cctor
    ssl_object::ssl_object(
        const ssl_object& _rhs ) :
        network_object( _rhs ),
        ssl_ctx_( _rhs.ssl_ctx() ),
        ssl_( _rhs.ssl() ),
        host_( _rhs.host() ),
        key_size_( _rhs.key_size() ),
        salt_size_( _rhs.salt_size() ),
        num_hash_rounds_( _rhs.num_hash_rounds() ),
        encryption_algorithm_( _rhs.encryption_algorithm() ) {
    } // cctor

// =-=-=-=-=-=-=-
// public - dtor
    ssl_object::~ssl_object() {
    } // dtor

// =-=-=-=-=-=-=-
// public - assignment operator
    ssl_object& ssl_object::operator=(
        const ssl_object& _rhs ) {
        network_object::operator=( _rhs );
        ssl_ctx_                = _rhs.ssl_ctx();
        ssl_                    = _rhs.ssl();
        host_                   = _rhs.host();
        key_size_               = _rhs.key_size();
        salt_size_              = _rhs.salt_size();
        num_hash_rounds_        = _rhs.num_hash_rounds();
        encryption_algorithm_   = _rhs.encryption_algorithm();

        return *this;

    } // operator=

// =-=-=-=-=-=-=-
// public - equality operator
    bool ssl_object::operator==(
        const ssl_object& _rhs ) const {

        bool ret = network_object::operator==( _rhs );

        ret &= ( ssl_ctx_               == _rhs.ssl_ctx() );
        ret &= ( ssl_                   == _rhs.ssl() );
        ret &= ( host_                  == _rhs.host() );
        ret &= ( key_size_              == _rhs.key_size() );
        ret &= ( salt_size_             == _rhs.salt_size() );
        ret &= ( num_hash_rounds_       == _rhs.num_hash_rounds() );
        ret &= ( encryption_algorithm_  == _rhs.encryption_algorithm() );

        return ret;

    } // operator==

// =-=-=-=-=-=-=-
// public - resolver for ssl_manager
    error ssl_object::resolve(
        const std::string& _interface,
        plugin_ptr&        _ptr ) {
        // =-=-=-=-=-=-=-
        // check the interface type and error out if it
        // isn't a network interface
        if ( NETWORK_INTERFACE != _interface ) {
            std::stringstream msg;
            msg << "ssl_object does not support a [";
            msg << _interface;
            msg << "] plugin interface";
            return ERROR( SYS_INVALID_INPUT_PARAM, msg.str() );

        }

        // =-=-=-=-=-=-=-
        // ask the network manager for a SSL resource
        network_ptr net_ptr;
        std::string empty_context{};
        error ret = netwk_mgr.get_plugin(
            ProcessType,
            SSL_NETWORK_PLUGIN,
            SSL_NETWORK_PLUGIN,
            empty_context,
            net_ptr);
        if (!ret.ok()) {
            return PASS(ret);
        }

        // =-=-=-=-=-=-=-
        // upcast for out variable
        _ptr = boost::dynamic_pointer_cast< plugin_base >( net_ptr );

        return SUCCESS();

    } // resolve

// =-=-=-=-=-=-=-
// accessor for rule engine variables
    error ssl_object::get_re_vars(
        rule_engine_vars_t& _kvp ) {
        network_object::get_re_vars( _kvp );

        _kvp[SSL_HOST_KW] = host_.c_str();

        // =-=-=-=-=-=-=-
        // since the shared secret is random and unsigned it needs
        // a bit of sanitizaiton until we can copy it to the kvp
        std::stringstream key_sz;
        key_sz << key_size_;
        _kvp[SSL_KEY_SIZE_KW] = key_sz.str().c_str();

        std::stringstream salt_sz;
        salt_sz << salt_size_;
        _kvp[SSL_SALT_SIZE_KW] = salt_sz.str().c_str();

        std::stringstream hash_rnds;
        hash_rnds << num_hash_rounds_;
        _kvp[SSL_NUM_HASH_ROUNDS_KW] = hash_rnds.str().c_str();

        _kvp[SSL_ALGORITHM_KW] = encryption_algorithm_.c_str();

        return SUCCESS();

    } // get_re_vars

// =-=-=-=-=-=-=-
// conversion to client comm ptr
    error ssl_object::to_client( rcComm_t* _comm ) {
        if ( !_comm ) {
            return ERROR( SYS_INVALID_INPUT_PARAM, "null comm ptr" );
        }

        network_object::to_client( _comm );

        _comm->ssl     = ssl_;
        _comm->ssl_ctx = ssl_ctx_;
        std::copy(
            shared_secret_.begin(),
            shared_secret_.end(),
            &_comm->shared_secret[0] );
        _comm->key_size        = key_size_;
        _comm->salt_size       = salt_size_;
        _comm->num_hash_rounds = num_hash_rounds_;
        snprintf( _comm->encryption_algorithm, sizeof( _comm->encryption_algorithm ),
                  "%s", encryption_algorithm_.c_str() );

        return SUCCESS();

    } // to_client

// =-=-=-=-=-=-=-
// conversion to client comm ptr
    error ssl_object::to_server( rsComm_t* _comm ) {
        if ( !_comm ) {
            return ERROR( SYS_INVALID_INPUT_PARAM, "null comm ptr" );
        }

        network_object::to_server( _comm );

        _comm->ssl     = ssl_;
        _comm->ssl_ctx = ssl_ctx_;
        std::copy(
            shared_secret_.begin(),
            shared_secret_.end(),
            &_comm->shared_secret[0] );
        _comm->key_size        = key_size_;
        _comm->salt_size       = salt_size_;
        _comm->num_hash_rounds = num_hash_rounds_;
        snprintf( _comm->encryption_algorithm, sizeof( _comm->encryption_algorithm ),
                  "%s", encryption_algorithm_.c_str() );


        return SUCCESS();

    } // to_server


}; // namespace irods



