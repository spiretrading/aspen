import unittest

from fixtures import aspen, drive


class TestRange(unittest.TestCase):
  def test_ascending_range(self):
    reactor = aspen.range(0, 3)
    self.assertEqual(reactor.commit(0), aspen.State.CONTINUE_EVALUATED)
    self.assertEqual(reactor.eval(), 0)
    self.assertEqual(reactor.commit(1), aspen.State.CONTINUE_EVALUATED)
    self.assertEqual(reactor.eval(), 1)
    self.assertEqual(reactor.commit(2), aspen.State.COMPLETE_EVALUATED)
    self.assertEqual(reactor.eval(), 2)

  def test_descending_range(self):
    self.assertEqual(drive(aspen.range(3, 0, -1), 8), [3, 2, 1])

  def test_step(self):
    self.assertEqual(drive(aspen.range(0, 10, 3), 8), [0, 3, 6, 9])

  def test_stop_below_the_start(self):
    self.assertEqual(aspen.range(10, 9).commit(0), aspen.State.COMPLETE)

  def test_fractional_step(self):
    self.assertEqual(drive(aspen.range(0.0, 1.0, 0.5), 8), [0.0, 0.5])


if __name__ == '__main__':
  unittest.main()
