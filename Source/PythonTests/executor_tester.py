import threading
import unittest

from fixtures import aspen


class TestExecutor(unittest.TestCase):
  def test_run_until_none(self):
    commits = []
    reactor = aspen.lift(lambda value: commits.append(value), aspen.constant(1))
    aspen.Executor(reactor).run_until_none()
    self.assertEqual(commits, [1])

  def test_aborting_before_a_run(self):
    commits = []
    reactor = aspen.lift(lambda value: commits.append(value), aspen.Cell(0))
    executor = aspen.Executor(reactor)
    executor.abort()
    executor.run_until_complete()
    self.assertEqual(commits, [])

  def test_aborting_a_run(self):
    executor = aspen.Executor(aspen.Cell(0))
    abort = executor.abort
    runner = threading.Thread(target=executor.run_until_complete, daemon=True)
    runner.start()
    abort()
    runner.join(timeout=10)
    self.assertFalse(runner.is_alive())


if __name__ == '__main__':
  unittest.main()
