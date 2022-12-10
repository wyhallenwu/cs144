#include "stream_reassembler.hh"

#include <cstdio>
#include <utility>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity)
    , _capacity(capacity)
    , _first_unassembled_index(0)
    , _first_unaccepted_index(_capacity)
    , _unass_window({})
    , _eof_index(0)
    , _eof(false) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.

void StreamReassembler::store_unass_buffer(size_t index, const std::string &data) {
    size_t internal_index = 0;
    while (index < _first_unaccepted_index && internal_index < data.size()) {
        _unass_window.emplace(std::make_pair(index, data[internal_index]));
        index++;
        internal_index++;
    }
}

size_t StreamReassembler::expected_index() const { return _first_unassembled_index; }

std::string StreamReassembler::combine_new_data(size_t index, const std::string &data) {
    // search in the window and form a continuous string with end_index < _first_unaccepted_index
    std::string combined_data =
        data.substr(_first_unassembled_index - index, _first_unaccepted_index - _first_unassembled_index);
    size_t expected_index = _first_unassembled_index + combined_data.size();
    for (auto iter = _unass_window.begin(); iter != _unass_window.end();) {
        if (iter->first < expected_index) {
            iter = _unass_window.erase(iter);
        } else if (iter->first == expected_index) {
            combined_data += iter->second;
            iter = _unass_window.erase(iter);
            expected_index++;
        } else if (iter->first > expected_index) {
            break;
        }
    }
    return combined_data;
}

void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // if receive the last piece of stream, initialize the end sign
    if (eof) {
        _eof = true;
        _eof_index = index + data.size();
    }
    // update the bound of the window
    _first_unaccepted_index = _first_unassembled_index + _capacity - _output.buffer_size();
    // deal with different conditions
    if (index + data.size() < _first_unassembled_index || index >= _first_unaccepted_index) {
        return;
    } else if (index > _first_unassembled_index) {
        store_unass_buffer(index, data);
    } else {
        std::string to_write_str = combine_new_data(index, data);
        size_t num_bytes = _output.write(to_write_str);
        _first_unassembled_index += num_bytes;
    }
    // current stream over
    if (_eof && _first_unassembled_index == _eof_index) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unass_window.size(); }

bool StreamReassembler::empty() const { return _output.buffer_empty() && _unass_window.empty(); }
