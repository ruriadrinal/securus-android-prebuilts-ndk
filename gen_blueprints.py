#!/usr/bin/env python
#
# Copyright 2015 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Generates the Android.bp file for prebuilts/ndk."""
import os


def local_path(path):
    """Returns an abspath to the given path from this file's directory."""
    return os.path.normpath(os.path.join(os.path.dirname(__file__), path))


def find(path, names):
    """Finds a list of files in a directory that match the given names."""
    found = []
    for root, _, files in os.walk(path):
        for file_name in sorted(files):
            if file_name in names:
                abspath = os.path.abspath(os.path.join(root, file_name))
                rel_to_root = abspath.replace(os.path.abspath(path), '')
                found.append(rel_to_root[1:])  # strip leading /
    return found


def sdk_version_from_path(path):
    """Returns the integer SDK version for the given path."""
    return int(path.split('/')[0].split('-')[1])


def get_prebuilts(names):
    """Returns a list of prebuilt objects that match the given names."""
    prebuilts_path = local_path('current/platforms')
    prebuilts = find(prebuilts_path, names)
    prebuilts = [p for p in prebuilts if 'arch-arm/' in p]
    prebuilts.sort(key=sdk_version_from_path)
    return prebuilts


def gen_crt_prebuilt(_, name, version):
    """Generate a module for a CRT prebuilt object."""
    return ('ndk_prebuilt_object {{\n'
            '    name: "ndk_{name}.{version}",\n'
            '    sdk_version: "{version}",\n'
            '}}'.format(name=name, version=version))


def gen_prebuilts(module_generator, names):
    """Generate blueprints for the given modules."""
    prebuilts = []
    for prebuilt in get_prebuilts(names):
        name = os.path.splitext(os.path.basename(prebuilt))[0]
        version = sdk_version_from_path(prebuilt)
        if version < 9:
            # We don't support anything before Gingerbread any more.
            continue
        prebuilts.append(module_generator(prebuilt, name, version))
    return prebuilts


def main():
    """Program entry point."""
    blueprints = gen_prebuilts(gen_crt_prebuilt, (
        'crtbegin_so.o',
        'crtend_so.o',
        'crtbegin_dynamic.o',
        'crtbegin_static.o',
        'crtend_android.o'))

    with open(local_path('Android.bp'), 'w') as bpfile:
        bpfile.write('// THIS FILE IS AUTOGENERATED BY gen-blueprints.py\n')
        bpfile.write('// DO NOT EDIT\n')
        bpfile.write('\n')
        bpfile.write('\n\n'.join(blueprints))
        bpfile.write('\n\n')
        bpfile.write('build = ["cpufeatures.bp", "stl.bp"]')


if __name__ == '__main__':
    main()
