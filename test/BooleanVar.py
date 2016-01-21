"""Module for testing boolean variables."""

class TestBooleanVar(BaseTestCase):

    def testBindFalse(self):
        "test binding in a False value"
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                (False,))
        self.assertEqual(result, "FALSE")

    def testBindNull(self):
        "test binding in a null value"
        self.cursor.setinputsizes(None, bool)
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                (None,))
        self.assertEqual(result, "NULL")

    def testBindOutFalse(self):
        "test binding out a boolean value (False)"
        result = self.cursor.callfunc("pkg_TestBooleans.IsLessThan10",
                cx_Oracle.BOOLEAN, (15,))
        self.assertEqual(result, False)

    def testBindOutTrue(self):
        "test binding out a boolean value (True)"
        result = self.cursor.callfunc("pkg_TestBooleans.IsLessThan10", bool,
                (5,))
        self.assertEqual(result, True)

    def testBindTrue(self):
        "test binding in a True value"
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                (True,))
        self.assertEqual(result, "TRUE")

