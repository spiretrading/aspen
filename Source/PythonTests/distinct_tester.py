import unittest

from fixtures import aspen, drive


class Value:
  def __init__(self, value):
    self.value = value

  def __hash__(self):
    return hash(self.value)

  def __eq__(self, other):
    return self.value == other.value


class Colliding:
  def __hash__(self):
    return 0

  def __eq__(self, other):
    raise ValueError('equality failed')


class TestDistinct(unittest.TestCase):
  def test_duplicate_values(self):
    queue = aspen.Queue()
    queue.push(1)
    queue.push(1)
    queue.push(2)
    self.assertEqual(drive(aspen.distinct(queue), 4), [1, 2])

  def test_equal_values(self):
    queue = aspen.Queue()
    queue.push(Value(1))
    queue.push(Value(1))
    queue.push(Value(2))
    evaluations = drive(aspen.distinct(queue), 4)
    self.assertEqual([value.value for value in evaluations], [1, 2])

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

  def test_raising_equality(self):
    queue = aspen.Queue()
    queue.push(Colliding())
    queue.push(Colliding())
    reactor = aspen.distinct(queue)
    self.assertTrue(aspen.has_evaluation(reactor.commit(0)))
    reactor.eval()
    self.assertTrue(aspen.has_evaluation(reactor.commit(1)))
    with self.assertRaises(ValueError):
      reactor.eval()


if __name__ == '__main__':
  unittest.main()
