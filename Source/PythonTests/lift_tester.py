import unittest

from fixtures import aspen


def failing(value):
  raise ValueError('evaluation failed')


class TestLift(unittest.TestCase):
  def test_no_arguments(self):
    reactor = aspen.lift(lambda: 512)
    self.assertEqual(reactor.commit(0), aspen.State.COMPLETE_EVALUATED)
    self.assertEqual(reactor.eval(), 512)

  def test_argument(self):
    reactor = aspen.lift(lambda value: value * 2, aspen.constant(21))
    self.assertEqual(reactor.commit(0), aspen.State.COMPLETE_EVALUATED)
    self.assertEqual(reactor.eval(), 42)

  def test_state_value(self):
    reactor = aspen.lift(lambda state: state,
      aspen.StateReactor(aspen.constant(1)))
    self.assertTrue(aspen.has_evaluation(reactor.commit(0)))
    self.assertEqual(reactor.eval(), aspen.State.COMPLETE_EVALUATED)

  def test_evaluation_without_a_value(self):
    queue = aspen.Queue()
    queue.push(1)
    queue.push(2)
    reactor = aspen.lift(
      lambda value: aspen.FunctionEvaluation(aspen.State.NONE)
        if value == 1 else value, queue)
    self.assertFalse(aspen.has_evaluation(reactor.commit(0)))
    self.assertTrue(aspen.has_evaluation(reactor.commit(1)))
    self.assertEqual(reactor.eval(), 2)

  def test_evaluation_with_a_state(self):
    reactor = aspen.lift(
      lambda value: aspen.FunctionEvaluation(value * 2, aspen.State.COMPLETE),
      aspen.constant(21))
    self.assertEqual(reactor.commit(0), aspen.State.COMPLETE_EVALUATED)
    self.assertEqual(reactor.eval(), 42)

  def test_evaluating_state_without_a_value(self):
    with self.assertRaises(ValueError):
      aspen.FunctionEvaluation(aspen.State.EVALUATED)

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
