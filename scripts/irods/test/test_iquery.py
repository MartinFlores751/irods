import json
import unittest

from . import session

rodsadmins = [('otherrods', 'rods')]
rodsusers  = [('alice', 'apass')]

class Test_IQuery(session.make_sessions_mixin(rodsadmins, rodsusers), unittest.TestCase):

    def setUp(self):
        super(Test_IQuery, self).setUp()
        self.admin = self.admin_sessions[0]
        self.user = self.user_sessions[0]

    def tearDown(self):
        super(Test_IQuery, self).tearDown()

    def test_iquery_can_run_a_query__issue_7570(self):
        json_string = json.dumps([[self.user.session_collection]])
        self.user.assert_icommand(
            ['iquery', f"select COLL_NAME where COLL_NAME = '{self.user.session_collection}'"], 'STDOUT', [json_string])

    def test_iquery_supports_reading_query_from_stdin__issue_7570(self):
        json_string = json.dumps([[self.user.session_collection]])
        self.user.assert_icommand(
            ['iquery'], 'STDOUT', [json_string], input=f"select COLL_NAME where COLL_NAME = '{self.user.session_collection}'")

    def test_iquery_returns_error_on_invalid_zone__issue_7570(self):
        ec, out, err = self.user.assert_icommand_fail(['iquery', '-z', 'invalid_zone', 'select COLL_NAME'], 'STDOUT')
        self.assertEqual(ec, 1)
        self.assertEqual(len(out), 0)
        self.assertEqual(err, 'error: -26000\n') # SYS_INVALID_ZONE_NAME

        ec, out, err = self.user.assert_icommand_fail(['iquery', '-z', '', 'select COLL_NAME'], 'STDOUT')
        self.assertEqual(ec, 1)
        self.assertEqual(len(out), 0)
        self.assertEqual(err, 'error: -26000\n') # SYS_INVALID_ZONE_NAME

        ec, out, err = self.user.assert_icommand_fail(['iquery', '-z', ' ', 'select COLL_NAME'], 'STDOUT')
        self.assertEqual(ec, 1)
        self.assertEqual(len(out), 0)
        self.assertEqual(err, 'error: -26000\n') # SYS_INVALID_ZONE_NAME

    def test_iquery_returns_error_on_invalid_query_tokens__issue_7570(self):
        ec, out, err = self.user.assert_icommand_fail(['iquery', ' '], 'STDOUT')
        self.assertEqual(ec, 1)
        self.assertEqual(len(out), 0)
        self.assertEqual(err, 'error: -167000\n') # SYS_LIBRARY_ERROR

        ec, out, err = self.user.assert_icommand_fail(['iquery', 'select'], 'STDOUT')
        self.assertEqual(ec, 1)
        self.assertEqual(len(out), 0)
        self.assertEqual(err, 'error: -167000\n') # SYS_LIBRARY_ERROR

        ec, out, err = self.user.assert_icommand_fail(['iquery', 'select INVALID_COLUMN'], 'STDOUT')
        self.assertEqual(ec, 1)
        self.assertEqual(len(out), 0)
        self.assertEqual(err, 'error: -130000\n') # SYS_INVALID_INPUT_PARAM

    def test_iquery_returns_error_when_closing_single_quote_is_missing__issue_6393(self):
        ec, out, err = self.user.assert_icommand_fail(
            ['iquery', f"select COLL_NAME where COLL_NAME = '{self.user.session_collection}"], 'STDOUT')
        self.assertEqual(ec, 1)
        self.assertEqual(len(out), 0)
        self.assertEqual(err, 'error: -167000\n') # SYS_LIBRARY_ERROR

    def test_iquery_distinguishes_embedded_IN_substring_from_IN_operator__issue_3064(self):
        attr_name = 'originalVersionId'
        attr_value = 'ignored'

        try:
            self.user.assert_icommand(['imeta', 'add', '-C', self.user.session_collection, attr_name, attr_value])

            json_string = json.dumps([[attr_name, attr_value]], separators=(',', ':'))
            for op in ['IN', 'in']:
                with self.subTest(f'op={op}'):
                    query_string = f"select META_COLL_ATTR_NAME, META_COLL_ATTR_VALUE where META_COLL_ATTR_NAME {op} ('{attr_name}')"
                    self.user.assert_icommand(['iquery', query_string], 'STDOUT', [json_string])

        finally:
            # Remove the metadata.
            self.user.assert_icommand(['imeta', 'rm', '-C', self.user.session_collection, attr_name, attr_value])

    def test_iquery_supports_OR_operator__issue_4069(self):
        query_string = f"select COLL_NAME where COLL_NAME = '{self.user.home_collection}' or COLL_NAME like '{self.admin.home_collection}' order by COLL_NAME"
        json_string = json.dumps([[self.user.home_collection], [self.admin.home_collection]], separators=(',', ':'))
        self.admin.assert_icommand(['iquery', query_string], 'STDOUT', json_string)

    def test_iquery_supports_embedded_single_quotes__issue_5727(self):
        # Create a collection containing an embedded single quote in the name.
        collection = "issue'5727.c"
        self.user.assert_icommand(['imkdir', collection])

        # Create a data object containing an embedded single quote in the name.
        data_object = "test_iquery_supports_embedded_single_quotes'__issue_5727"
        self.user.assert_icommand(['istream', 'write', data_object], input='data')

        # Show the GenQuery2 parser provides ways of dealing with embedded single quotes.
        escaped_strings = [
            data_object.replace("'", '\\x27'),  # Uses hex value to escape single quote.
            data_object.replace("'", "''")      # Uses two single quotes to escape single quote like in SQL.
        ]
        json_string = json.dumps([[data_object, self.user.session_collection]], separators=(',', ':'))
        for escaped in escaped_strings:
            query_string = f"select DATA_NAME, COLL_NAME where COLL_NAME = '{self.user.session_collection}' or DATA_NAME = '{escaped}'"
            with self.subTest(f'escaped_string={escaped}'):
                self.user.assert_icommand(['iquery', query_string], 'STDOUT', [json_string])

    def test_iquery_returns_error_on_invalid_query_string__issue_5734(self):
        # The following query is missing a closing single quote on the first condition
        # in the WHERE clause.
        query_string = "select DATA_ID, DATA_REPL_NUM, COLL_NAME, DATA_NAME, DATA_RESC_HIER, DATA_PATH, META_DATA_ATTR_VALUE " + \
            "where META_DATA_ATTR_NAME = 'id_foo and META_DATA_ATTR_VALUE in ('260,'3261852','3261856','3261901','3080907','3083125'," + \
            "'3083853','3084203','3085046','3091021','3092210','3092313','3093766','3094073','3094078','3095017','3095445','3095522','3097128'," + \
            "'3097225','3097311','3097480','3097702','3097750')"
        ec, out, err = self.user.assert_icommand_fail(['iquery', query_string], 'STDOUT')
        self.assertEqual(ec, 1)
        self.assertEqual(len(out), 0)
        self.assertEqual(err, 'error: -167000\n') # SYS_LIBRARY_ERROR

    def test_genquery2_maps_genquery_user_zone_columns_to_correct_database_columns__issue_8134_8135(self):
        # Show that the column mapping listing contains the correct mappings.
        _, out, _ = self.user.assert_icommand(['iquery', '-c'], 'STDOUT')
        collection_mapping_is_correct = False
        data_object_mapping_is_correct = False
        for line in out.splitlines():
            if '(R_USER_MAIN.zone_name)' in line:
                if 'COLL_ACCESS_USER_ZONE' in line: collection_mapping_is_correct = True
                elif 'DATA_ACCESS_USER_ZONE' in line: data_object_mapping_is_correct = True
        self.assertTrue(collection_mapping_is_correct)
        self.assertTrue(data_object_mapping_is_correct)

        # Show that usage of the GenQuery2 COLL permissions columns results in non-empty resultsets.
        query_string = f"select COLL_ACCESS_USER_NAME, COLL_ACCESS_USER_ZONE, COLL_ACCESS_PERM_NAME where COLL_NAME = '{self.user.home_collection}'"
        json_string = json.dumps([[self.user.username, self.user.zone_name, 'own']], separators=(',', ':'))
        self.user.assert_icommand(['iquery', query_string], 'STDOUT', [json_string])

        # Show that usage of the GenQuery2 DATA permissions columns results in non-empty resultsets.
        data_name = 'test_genquery2_maps_genquery_user_zone_columns_to_correct_database_columns__issue_8134_8135'
        self.user.assert_icommand(['itouch', data_name])
        query_string = f"select DATA_ACCESS_USER_NAME, DATA_ACCESS_USER_ZONE, DATA_ACCESS_PERM_NAME where DATA_NAME = '{data_name}'"
        json_string = json.dumps([[self.user.username, self.user.zone_name, 'own']], separators=(',', ':'))
        self.user.assert_icommand(['iquery', query_string], 'STDOUT', [json_string])

        # Show that use of the GenQuery2 COLL permissions columns in a GROUP BY clause works as well.
        query_string = f"select COLL_ACCESS_USER_NAME, COLL_ACCESS_USER_ZONE, count(COLL_ID) where COLL_NAME = '{self.user.home_collection}' group by COLL_ACCESS_USER_NAME, COLL_ACCESS_USER_ZONE"
        json_string = json.dumps([[self.user.username, self.user.zone_name, '1']], separators=(',', ':'))
        self.user.assert_icommand(['iquery', query_string], 'STDOUT', [json_string])

        # Show that use of the GenQuery2 DATA permissions columns in a GROUP BY clause works as well.
        query_string = f"select DATA_ACCESS_USER_NAME, DATA_ACCESS_USER_ZONE, count(DATA_ID) where DATA_NAME = '{data_name}' group by DATA_ACCESS_USER_NAME, DATA_ACCESS_USER_ZONE"
        json_string = json.dumps([[self.user.username, self.user.zone_name, '1']], separators=(',', ':'))
        self.user.assert_icommand(['iquery', query_string], 'STDOUT', [json_string])
