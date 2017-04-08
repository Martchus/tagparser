#!/bin/bash
if [[ ! $@ ]]; then
    echo 'Prints the list of testfiles with their sha256 sums required by the projects in the specified directories.'
    echo 'However, no project directories have been specified.'
    exit -1
fi

if [[ ! -d $TEST_FILE_PATH ]]; then
    echo 'Env variable TEST_FILE_PATH does not point to local directory.'
    exit -2
fi

declare -A test_files
declare -A ignored_files

# iterate over specified source directories
for srcdir in "$@"; do
    # find icons in *.cpp files
    for filename_from_call in $(find "$srcdir" -iname '*.cpp' -print0 | xargs -0 cat | grep -Po 'testFilePath\(.*?\)'); do
        [ "${filename_from_call: 13 : 1}" == '"' ] \
            && test_files["${filename_from_call: 13 : -1}"]=1 \
            || ignored_files["${filename_from_call: 13 : -1}"]=1
    done
    for filename_from_call in $(find "$srcdir" -iname '*.cpp' -print0 | xargs -0 cat | grep -Po 'workingCopyPath\(.*?\)'); do
        [ "${filename_from_call: 16 : 1}" == '"' ] \
            && test_files["${filename_from_call: 16 : -1}"]=1 \
            || ignored_files["${filename_from_call: 16 : -1}"]=1
    done
done

# print ignored files
echo 'Ignored values (not a string literal):'
for ignored_file in "${!ignored_files[@]}"; do
    echo "${ignored_file}"
done

# print results in alphabetical order
sorted_file_names=($(echo "${!test_files[@]}" | tr ' ' '\n' | sort))
echo
echo 'const testFiles[] = {'
for test_file in "${sorted_file_names[@]}"; do
  echo "    {${test_file}, {\"$(sha256sum "${TEST_FILE_PATH}/${test_file: 1 : -1}" | awk '{ print $1 }')\"}},"
done
echo '};'
