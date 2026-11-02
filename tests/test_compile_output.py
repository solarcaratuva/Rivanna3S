import contextlib
import importlib.util
import io
import os
from pathlib import Path
import sys
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('compile_script', Path(__file__).resolve().parents[1] / 'compile.py')
compiler = importlib.util.module_from_spec(spec)
spec.loader.exec_module(compiler)


class OutputTests(unittest.TestCase):
    def test_empty_output_has_no_sections(self):
        self.assertEqual(compiler.process_compilation_output('ninja: no work to do.\n'), ('', False))

    def test_diagnostics_keep_order_context_and_notes(self):
        warning = 'a.cpp:4:2: warning: unused variable\n    4 | int a;\n      |     ^\n'
        output, has_errors = compiler.process_compilation_output(
            warning + 'b.cpp:2: warning: another warning\n' + warning +
            'a.cpp:3:1: note: declared here\n' +
            'ld: object.o: note: linker context\n')
        self.assertEqual(output.count('unused variable'), 1)
        self.assertLess(output.index('a.cpp:4'), output.index('b.cpp:2'))
        self.assertIn('4 | int a;', output)
        self.assertIn('note: declared here', output)
        self.assertIn('note: linker context', output)
        self.assertNotIn('errors', output)
        self.assertFalse(has_errors)

    def test_fatal_and_linker_errors(self):
        output, has_errors = compiler.process_compilation_output(
            '\033[31ma.cpp:1: fatal error: missing.h: No such file\033[0m\n'
            '/usr/bin/ld: object.o: undefined reference to `missing`\n'
            '/usr/bin/ld: region `FLASH` overflowed by 24 bytes\n')
        self.assertTrue(has_errors)
        self.assertIn('Compiler errors', output)
        self.assertIn('Linker errors', output)
        self.assertIn('overflowed', output)
        self.assertNotIn('\033', output)

    def run_build(self, output, code=0, **options):
        previous = Path.cwd()
        with tempfile.TemporaryDirectory() as directory:
            try:
                os.chdir(directory)
                captured = io.StringIO()
                with contextlib.redirect_stdout(captured):
                    result = compiler.run_compilation(
                        [sys.executable, '-c', f'import sys; print({output!r}); sys.exit({code})'], **options)
                raw_log = Path('build/compile.log').read_text()
                return result, captured.getvalue(), raw_log
            finally:
                os.chdir(previous)

    def test_progress_and_silent(self):
        data = '[1/2] Building object\na.cpp:1: warning: example'
        for silent in (False, True):
            code, shown, raw = self.run_build(data, silent=silent)
            self.assertEqual(code, 0)
            self.assertEqual('[1/2]' in shown, not silent)
            self.assertIn('warning: example', shown)
            self.assertIn('[1/2]', raw)

    def test_unknown_failure_is_not_hidden_by_warnings(self):
        code, shown, raw = self.run_build('a.cpp:1: warning: example\nninja: unknown target', code=7, silent=True)
        self.assertEqual(code, 7)
        self.assertIn('ninja: unknown target', shown)
        self.assertIn('Compilation failed.', shown)

    def test_section_titles_in_warning_do_not_hide_failure(self):
        for title in ('Compiler errors', 'Linker errors'):
            with self.subTest(title=title):
                data = f'a.cpp:1: warning: {title} explained here\nninja: unknown target'
                code, shown, raw = self.run_build(data, code=1, silent=True)
                self.assertEqual(code, 1)
                self.assertIn('ninja: unknown target', shown)

    def test_raw_output_is_not_filtered(self):
        data = 'unrecognized output\na.cpp:1: error: example'
        code, shown, raw = self.run_build(data, code=1, raw=True)
        self.assertEqual(code, 1)
        self.assertIn(data, shown)
        self.assertEqual(raw, data + '\n')
        self.assertNotIn('Compiler errors', shown)


if __name__ == '__main__':
    unittest.main()
