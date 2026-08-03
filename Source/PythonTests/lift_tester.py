import unittest

from fixtures import aspen


def failing(value):
  raise ValueError('evaluation failed')


class Lift(unittest.TestCase):
  def test_no_arguments(self):
    reactor = aspen.lift(lambda: 512)
    self.assertEqual(reactor.commit(0), aspen.State.COMPLETE_EVALUATED)
    self.assertEqual(reactor.eval(), 512)

  def test_argument(self):
    reactor = aspen.lift(lambda value: value * 2, aspen.constant(21))
    self.assertEqual(reactor.commit(0), aspen.State.COMPLETE_EVALUATED)
    self.assertEqual(reactor.eval(), 42)

  def test_raising_evaluation(self):
    reactor = aspen.lift(failing, aspen.constant(1))
    self.assertTrue(aspen.has_evaluation(reactor.commit(0)))
    with self.assertRaises(ValueError):
      reactor.eval()

  def test_repeated_raising_evaluation(self):
    reactor = aspen.lift(failing, aspen.constant(1))
    reactor.commit(0)
    with self.assertRaises(ValueError):
      reactor.eval()
    with self.assertRaises(ValueError):
      reactor.eval()

  def test_shared_raising_evaluation(self):
    source = aspen.lift(failing, aspen.constant(1))
    left = aspen.lift(lambda value: value, source)
    right = aspen.lift(lambda value: value, source)
    left.commit(0)
    right.commit(0)
    with self.assertRaises(ValueError):
      left.eval()
    with self.assertRaises(ValueError):
      right.eval()


if __name__ == '__main__':
  unittest.main()
