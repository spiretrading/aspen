import unittest

from fixtures import aspen


class Traced(aspen.Box):
  def __init__(self, reactor):
    super().__init__(reactor)
    self.commits = 0

  def commit(self, sequence):
    self.commits += 1
    return super().commit(sequence)


class TestProxy(unittest.TestCase):
  def test_reactor(self):
    reactor = aspen.proxy()
    reactor.set_reactor(aspen.constant(5))
    self.assertTrue(aspen.has_evaluation(reactor.commit(0)))
    self.assertEqual(reactor.eval(), 5)

  def test_boxed_override(self):
    traced = Traced(aspen.constant(1))
    aspen.Box(traced).commit(0)
    self.assertEqual(traced.commits, 1)

  def test_proxied_override(self):
    traced = Traced(aspen.constant(1))
    reactor = aspen.proxy()
    reactor.set_reactor(traced)
    reactor.commit(0)
    self.assertEqual(traced.commits, 1)


if __name__ == '__main__':
  unittest.main()
