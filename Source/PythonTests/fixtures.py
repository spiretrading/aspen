import aspen


def drive(reactor, count):
  evaluations = []
  for sequence in range(count):
    state = reactor.commit(sequence)
    if aspen.has_evaluation(state):
      evaluations.append(reactor.eval())
    if aspen.is_complete(state):
      break
  return evaluations
