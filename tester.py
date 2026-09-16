import argparse
import importlib.machinery
import importlib.util
import os
import sys
import unittest

ROOT = os.path.dirname(os.path.abspath(__file__))
DIRECTORY = os.path.join(ROOT, 'Source', 'PythonTests')
CONFIGURATIONS = ('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')
MODULES = ('aspen.pyd', 'aspen.so')


def _roots():
  roots = []
  for root in (os.getcwd(), ROOT):
    root = os.path.abspath(root)
    if root not in roots:
      roots.append(root)
  return roots


def _built_module(configuration):
  for root in _roots():
    selected = configuration
    if not selected:
      config = os.path.join(root, 'CMakeFiles', 'config.txt')
      if not os.path.isfile(config):
        continue
      with open(config) as source:
        selected = source.read().strip()
    if selected not in CONFIGURATIONS:
      return None
    for module in MODULES:
      path = os.path.join(root, 'Libraries', selected, module)
      if os.path.isfile(path):
        return path
    if not configuration:
      return None
  return None


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('prefix', nargs='?', help='Test filename prefix.')
  parser.add_argument('--config', choices=CONFIGURATIONS,
    help='Build configuration (defaults to the saved build configuration).')
  args = parser.parse_args()
  if args.prefix:
    pattern = '{}*.py'.format(args.prefix)
  else:
    pattern = '*_tester.py'
  module = _built_module(args.config)
  if not module:
    print('No Aspen module found for the selected build configuration. '
      'Run build or specify --config.', file=sys.stderr)
    return 1
  loader = importlib.machinery.ExtensionFileLoader('aspen', module)
  spec = importlib.util.spec_from_file_location('aspen', module, loader=loader)
  aspen = importlib.util.module_from_spec(spec)
  sys.modules['aspen'] = aspen
  loader.exec_module(aspen)
  sys.path.insert(0, DIRECTORY)
  print('Testing {}'.format(aspen.__file__), flush=True)
  tests = unittest.defaultTestLoader.discover(DIRECTORY, pattern=pattern)
  if tests.countTestCases() == 0:
    print('No tests matched {}.'.format(pattern))
    return 1
  result = unittest.TextTestRunner(verbosity=2).run(tests)
  return 0 if result.wasSuccessful() else 1


if __name__ == '__main__':
  sys.exit(main())
