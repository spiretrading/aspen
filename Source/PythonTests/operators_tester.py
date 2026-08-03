import decimal
import unittest

from fixtures import aspen


class TestOperators(unittest.TestCase):
  def test_modulo(self):
    reactor = aspen.constant(7) % aspen.constant(4)
    self.assertTrue(aspen.has_evaluation(reactor.commit(0)))
    self.assertEqual(reactor.eval(), 3)

  def test_reflected_modulo(self):
    reactor = aspen.constant(decimal.Decimal(7)) % aspen.constant(4)
    self.assertTrue(aspen.has_evaluation(reactor.commit(0)))
    self.assertEqual(reactor.eval(), decimal.Decimal(3))

  def test_unsupported_modulo(self):
    reactor = aspen.constant(decimal.Decimal(1)) % aspen.constant(1.5)
    self.assertTrue(aspen.has_evaluation(reactor.commit(0)))
    with self.assertRaises(TypeError):
      reactor.eval()

  def test_modulo_of_an_unsupported_type(self):
    reactor = aspen.constant(None) % aspen.constant(1)
    self.assertTrue(aspen.has_evaluation(reactor.commit(0)))
    with self.assertRaises(TypeError):
      reactor.eval()


if __name__ == '__main__':
  unittest.main()
