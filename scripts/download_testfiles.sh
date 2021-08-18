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

# find source location
srcdir=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
if ! [[ -d $srcdir/testfiles ]]; then
    fail "The tagparser checkout \"$srcdir\" does not contain a testfiles directory."
    exit -2
fi

# read args
testfilespath="$1"
testfileurl="$2"
if [[ -z $testfilespath ]] || [[ -z $testfileurl ]]; then
    echo "Usage: testfilepath testfileurl"
    echo " testfilepath specifies the directory to store the downloaded files"
    echo " testfileurl specifies the URL to the server hosting the test files"
    exit -1
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
    'mp4/android-8.1-camera-recoding.mp4'
    'mtx-test-data/aac/he-aacv2-ps.m4a'
    'mtx-test-data/alac/othertest-itunes.m4a'
    'mtx-test-data/mp3/id3-tag-and-xing-header.mp3'
    'mtx-test-data/mp4/10-DanseMacabreOp.40.m4a'
    'mtx-test-data/mp4/1080p-DTS-HD-7.1.mp4'
    'mtx-test-data/mp4/dash/dragon-age-inquisition-H1LkM6IVlm4-video.mp4'
    'mtx-test-data/ogg/qt4dance_medium.ogg'
    'mtx-test-data/opus/v-opus.ogg'
    'misc/multiple_id3v2_4_values.mp3'
    'ogg/noise-without-cover.opus'
    'ogg/noise-broken-segment-termination.opus'
    'ogg/example-cover.png'
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

# convert more testfiles from the downloaded ones (FIXME: use a Makefile)
inform "Creating further testfiles with ffmpeg and mkvtoolnix"
mkdir -p flac
mkdir -p mkv
convert() {
    local filename=$1 && shift
    if [[ -f $filename ]]; then
        inform "Skipping already existing $filename"
        return
    fi
    "$@"
}
convert flac/test.flac ffmpeg -i mtx-test-data/alac/othertest-itunes.m4a -c:a flac flac/test.flac
convert flac/test.ogg ffmpeg -i flac/test.flac -vn -c:a copy flac/test.ogg
convert mkv/av1_test.mkv ffmpeg -i matroska_wave1/test1.mkv -t 1 -c:v libaom-av1 -crf 30 -cpu-used 5 -an -strict experimental mkv/av1_test.mkv
convert mp4/av1_test.mp4 ffmpeg -i mkv/av1_test.mkv -c copy mp4/av1_test.mp4
convert mp4/test1-flac.m4a ffmpeg -i mp4/test1.m4a -c:a flac -f mp4  -strict -2 mp4/test1-flac.m4a
convert mp4/test1-opus.m4a ffmpeg -i mp4/test1.m4a -c:a libopus -f mp4  -strict -2 mp4/test1-opus.m4a
convert mp4/test1-vorbis.m4a ffmpeg -i mp4/test1.m4a -c:a libvorbis -f mp4  -strict -2 mp4/test1-vorbis.m4a
convert misc/av1.ivf ffmpeg -i mkv/av1_test.mkv -c copy misc/av1.ivf
convert mkv/nested-tags.mkv \
        mkvmerge --ui-language en_US \
                 --output 'mkv/nested-tags.mkv' \
                 --no-global-tags \
                 --language '0:und' \
                 --default-track '0:yes' \
                 --language '1:und' \
                 --default-track '1:yes' \
                 \( 'mtx-test-data/mkv/tags.mkv' \) \
                 --global-tags "$srcdir/testfiles/mkv/nested-tags.xml" \
                 --track-order '0:0,0:1'
convert mp4/chapters.m4b ffmpeg -i "$srcdir/testfiles/metadata_for_ffmpeg.txt" -i flac/test.flac -map 1:0 -map_metadata 1 -c:a aac -ac 2 mp4/chapters.m4b

success "All testfiles downloaded/converted!"
