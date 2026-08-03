import unittest

from fixtures import aspen, drive


class TestDistinct(unittest.TestCase):
  def test_duplicate_values(self):
    queue = aspen.Queue()
    queue.push(1)
    queue.push(1)
    queue.push(2)
    self.assertEqual(drive(aspen.distinct(queue), 4), [1, 2])

  def test_unhashable_values(self):
    queue = aspen.Queue()
    queue.push([1])
    reactor = aspen.distinct(queue)
    self.assertTrue(aspen.has_evaluation(reactor.commit(0)))
    with self.assertRaises(TypeError):
      reactor.eval()

  def test_raising_hash(self):
    class Hashing:
      def __hash__(self):
        raise ValueError('hash failed')

    queue = aspen.Queue()
    queue.push(Hashing())
    reactor = aspen.distinct(queue)
    self.assertTrue(aspen.has_evaluation(reactor.commit(0)))
    with self.assertRaises(ValueError):
      reactor.eval()


if __name__ == '__main__':
  unittest.main()
