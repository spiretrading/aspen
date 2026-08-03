import os
import signal
import sys
import threading
import unittest

from fixtures import aspen


class TestExecutor(unittest.TestCase):
  def test_run_until_none(self):
    commits = []
    reactor = aspen.lift(lambda value: commits.append(value), aspen.constant(1))
    aspen.Executor(reactor).run_until_none()
    self.assertEqual(commits, [1])

  def test_run_until_complete(self):
    commits = []
    reactor = aspen.lift(lambda value: commits.append(value), aspen.constant(1))
    aspen.Executor(reactor).run_until_complete()
    self.assertEqual(commits, [1])

  def test_repeated_runs(self):
    commits = []
    cell = aspen.Cell(1)
    executor = aspen.Executor(
      aspen.lift(lambda value: commits.append(value), cell))
    executor.run_until_none()
    cell.set_complete(2)
    executor.run_until_complete()
    self.assertEqual(commits, [1, 2])

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

  @unittest.skipIf(sys.platform == 'win32', 'requires posix signals')
  def test_interrupting_a_run(self):
    started = threading.Event()
    cell = aspen.Cell(0)
    executor = aspen.Executor(aspen.lift(lambda value: started.set(), cell))

    def interrupt():
      started.wait()
      os.kill(os.getpid(), signal.SIGINT)

    interrupter = threading.Thread(target=interrupt, daemon=True)
    interrupter.start()
    with self.assertRaises(KeyboardInterrupt):
      executor.run_until_complete()
    interrupter.join(timeout=10)

  @unittest.skipIf(sys.platform == 'win32', 'requires posix signals')
  def test_interrupting_a_run_with_a_handler(self):
    started = threading.Event()
    interrupts = []
    cell = aspen.Cell(0)
    executor = aspen.Executor(aspen.lift(lambda value: started.set(), cell))

    def interrupt():
      started.wait()
      os.kill(os.getpid(), signal.SIGINT)

    previous = signal.signal(signal.SIGINT, lambda *_: interrupts.append(1))
    try:
      interrupter = threading.Thread(target=interrupt, daemon=True)
      interrupter.start()
      executor.run_until_complete()
      interrupter.join(timeout=10)
    finally:
      signal.signal(signal.SIGINT, previous)
    self.assertEqual(interrupts, [1])


if __name__ == '__main__':
  unittest.main()
