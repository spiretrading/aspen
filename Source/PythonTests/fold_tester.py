import unittest

from fixtures import aspen, drive


class TestFold(unittest.TestCase):
  def test_series(self):
    queue = aspen.Queue()
    queue.push(1)
    queue.push(2)
    queue.push(3)
    self.assertEqual(drive(aspen.fold(lambda a, b: a + b, queue), 4), [3, 6])

  def test_make_fold_argument(self):
    self.assertTrue(hasattr(aspen, 'make_fold_argument'))

  def test_evaluator(self):
    left = aspen.FoldArgument()
    right = aspen.FoldArgument()
    evaluator = aspen.lift(lambda a, b: a + b, left, right)
    queue = aspen.Queue()
    queue.push(1)
    queue.push(2)
    queue.push(3)
    self.assertEqual(
      drive(aspen.fold(evaluator, left, right, queue), 4), [3, 6])

  def test_constructing_a_fold(self):
    left = aspen.FoldArgument()
    right = aspen.FoldArgument()
    evaluator = aspen.lift(lambda a, b: a + b, left, right)
    queue = aspen.Queue()
    queue.push(1)
    queue.push(2)
    self.assertEqual(drive(aspen.Fold(evaluator, left, right, queue), 4), [3])


if __name__ == '__main__':
  unittest.main()
