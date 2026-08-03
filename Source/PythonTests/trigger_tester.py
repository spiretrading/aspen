import unittest

from fixtures import aspen


class TestTrigger(unittest.TestCase):
  def test_signal(self):
    aspen.Trigger().signal()

  def test_slot(self):
    signals = []
    trigger = aspen.Trigger(lambda: signals.append(1))
    trigger.signal()
    self.assertEqual(signals, [1])

  def test_raising_slot(self):
    def failing():
      raise ValueError('slot failed')

    trigger = aspen.Trigger(failing)
    trigger.signal()


if __name__ == '__main__':
  unittest.main()
