import unittest

from fixtures import aspen


class Failing:
  def commit(self, sequence):
    raise ValueError('commit failed')

  def eval(self):
    return 0


class Untyped:
  def commit(self, sequence):
    return 1

  def eval(self):
    return 0


class Counting:
  def __init__(self):
    self.commits = 0

  def commit(self, sequence):
    self.commits += 1
    return aspen.State.COMPLETE_EVALUATED

  def eval(self):
    return self.commits


class PythonReactor(unittest.TestCase):
  def test_commit(self):
    reactor = aspen.Box(Counting())
    self.assertEqual(reactor.commit(0), aspen.State.COMPLETE_EVALUATED)
    self.assertEqual(reactor.eval(), 1)

  def test_raising_commit(self):
    reactor = aspen.Box(Failing())
    self.assertTrue(aspen.has_evaluation(reactor.commit(0)))
    with self.assertRaises(ValueError):
      reactor.eval()
    with self.assertRaises(ValueError):
      reactor.eval()

  def test_commit_returning_a_non_state(self):
    reactor = aspen.Box(Untyped())
    self.assertTrue(aspen.has_evaluation(reactor.commit(0)))
    with self.assertRaises(RuntimeError):
      reactor.eval()


class NonReactorArgument(unittest.TestCase):
  def test_concur(self):
    with self.assertRaises(TypeError):
      aspen.concur(5)

  def test_concat(self):
    with self.assertRaises(TypeError):
      aspen.concat(5)

  def test_override(self):
    with self.assertRaises(TypeError):
      aspen.override(5)


if __name__ == '__main__':
  unittest.main()
