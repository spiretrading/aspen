import threading
import unittest

from fixtures import aspen

ITERATIONS = 2000


class Yielding:
  def __del__(self):
    for _ in range(100000):
      pass


class TestCell(unittest.TestCase):
  def test_set(self):
    cell = aspen.Cell(1)
    self.assertTrue(aspen.has_evaluation(cell.commit(0)))
    self.assertEqual(cell.eval(), 1)
    cell.set(2)
    self.assertTrue(aspen.has_evaluation(cell.commit(1)))
    self.assertEqual(cell.eval(), 2)

  def test_setting_while_committing(self):
    cell = aspen.Cell(Yielding())
    executor = aspen.Executor(aspen.lift(lambda value: value, cell))
    runner = threading.Thread(target=executor.run_until_complete, daemon=True)
    runner.start()
    for _ in range(ITERATIONS):
      cell.set(Yielding())
    executor.abort()
    runner.join(timeout=30)
    self.assertFalse(runner.is_alive())


class TestQueue(unittest.TestCase):
  def test_push(self):
    queue = aspen.Queue()
    queue.push(1)
    queue.push(2)
    self.assertTrue(aspen.has_evaluation(queue.commit(0)))
    self.assertEqual(queue.eval(), 1)
    self.assertTrue(aspen.has_evaluation(queue.commit(1)))
    self.assertEqual(queue.eval(), 2)

  def test_pushing_while_committing(self):
    queue = aspen.Queue()
    queue.push(Yielding())
    executor = aspen.Executor(aspen.lift(lambda value: value, queue))
    runner = threading.Thread(target=executor.run_until_complete, daemon=True)
    runner.start()
    for _ in range(ITERATIONS):
      queue.push(Yielding())
    executor.abort()
    runner.join(timeout=30)
    self.assertFalse(runner.is_alive())


if __name__ == '__main__':
  unittest.main()
