#!/bin/sh -e

# define helper for colorful output
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
blue=$(tput setaf 4)
bold=$(tput bold)
normal=$(tput sgr0)
inform() {
    echo "${bold}==> ${blue}INFO:${normal} ${bold}${1}${normal}"
}
success() {
    echo "${bold}==> ${green}SUCCESS:${normal} ${bold}${1}${normal}"
}
fail() {
    echo "${bold}==> ${red}FAILURE:${normal} ${bold}${1}${normal}"
}

# read args
testfilespath="$1"
testfileurl="$2"
if [[ -z $testfilespath ]] || [[ -z $testfileurl ]]; then
    echo "Usage: testfilepath testfileurl"
    echo " testfilepath specifies the directory to store the downloaded files"
    echo " testfileurl specifies the URL to the server hosting the test files"
fi

# enter testfiles directory
if [[ -d $testfilespath ]]; then
    cd "$testfilespath"
else
    fail "specified testfiles directory does not exist"
    exit -1
fi

# define list of testfiles to be downloaded
testfiles=(
    'matroska_wave1/logo3_256x256.png'
    'matroska_wave1/test1.mkv'
    'matroska_wave1/test2.mkv'
    'matroska_wave1/test3.mkv'
    'matroska_wave1/test4.mkv'
    'matroska_wave1/test5.mkv'
    'matroska_wave1/test6.mkv'
    'matroska_wave1/test7.mkv'
    'matroska_wave1/test8.mkv'
    'mtx-test-data/mkv/handbrake-chapters-2.mkv'
    'mtx-test-data/mkv/tags.mkv'
    'mp4/test1.m4a'
    'mtx-test-data/aac/he-aacv2-ps.m4a'
    'mtx-test-data/alac/othertest-itunes.m4a'
    'mtx-test-data/mp3/id3-tag-and-xing-header.mp3'
    'mtx-test-data/mp4/10-DanseMacabreOp.40.m4a'
    'mtx-test-data/mp4/1080p-DTS-HD-7.1.mp4'
    'mtx-test-data/mp4/dash/dragon-age-inquisition-H1LkM6IVlm4-video.mp4'
    'mtx-test-data/ogg/qt4dance_medium.ogg'
    'mtx-test-data/opus/v-opus.ogg'
    'misc/multiple_id3v2_4_values.mp3'
)

# download the files
missing=()
for file in "${testfiles[@]}"; do
    if [[ -f $file ]]; then
        inform "Skipping already existing $file"
        continue
    fi

    dir=${file%/*}
    name=${file##*/}
    url=$testfileurl/$file

    inform "Downloading file $file from $url"
    mkdir -p "$dir"
    pushd "$dir"
    if ! wget --output-document="$name" "$url"; then
        rm "$name"
        missing+=("$file")
        fail "Unable to download $file"
    fi
    popd
done

# summarize missing files
if [[ ! -z $missing ]]; then
    echo
    fail "The following files could not be downloaded:"
    for file in "${missing[@]}"; do
        echo "$file"
    done
    exit 1
fi

# convert FLAC files for FLAC tests with ffmpeg
inform "Creating further testfiles with ffmpeg"
mkdir -p flac
# raw FLAC stream
[[ ! -f flac/test.flac ]] \
    && ffmpeg -i mtx-test-data/alac/othertest-itunes.m4a -c:a flac flac/test.flac \
    || inform "Skipping already existing flac/test.flac"
# FLAC in Ogg
[[ ! -f flac/test.ogg ]] \
    && ffmpeg -i flac/test.flac -c:a copy flac/test.ogg \
    || inform "Skipping already existing flac/test.ogg"

success "All testfiles downloaded/converted!"
