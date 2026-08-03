import keyword
import unittest

from fixtures import aspen


class TestNoneReactor(unittest.TestCase):
  def test_commit(self):
    self.assertEqual(aspen.none().commit(0), aspen.State.COMPLETE)

  def test_type_name(self):
    name = type(aspen.none()).__name__
    self.assertTrue(name.isidentifier())
    self.assertFalse(keyword.iskeyword(name))

  def test_module_attributes(self):
    for name in dir(aspen):
      self.assertFalse(keyword.iskeyword(name))


if __name__ == '__main__':
  unittest.main()
