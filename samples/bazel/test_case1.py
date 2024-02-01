import unittest


class TestCase1(unittest.TestCase):
    def testAssertTrue(self):
        self.assertTrue(1 + 1 == 2)

    def testAssertFalse(self):
        self.assertFalse(1 + 1 == 3)


if __name__ == "__main__":
    unittest.main()
