#include "tcp_receiver.hh"

#include "wrapping_integers.hh"

#include <optional>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    // the eof indicates that the current segment is the fin seg
    // it is different from the _fin flag because receiver may get more than one fin seg
    // if the current seg is not fin, it shouldn't set the push_substring()'s _end_index
    bool eof = false;
    // connection not set and receive the bebind sequence
    if (!_syn && !seg.header().syn) {
        return;
    }
    // initialize the isn when receive the syn
    if (!_syn && seg.header().syn) {
        _syn = true;
        _isn = seg.header().seqno;
        if (seg.header().fin) {
            _fin = eof = true;
        }
        _reassembler.push_substring(seg.payload().copy(), 0, eof);
        return;
    }
    // receive the fin and then set the eof to be true
    if (_syn && seg.header().fin) {
        _fin = eof = true;
    }
    uint64_t abs_seqno = unwrap(seg.header().seqno, _isn.value(), _reassembler.expected_index());
    _reassembler.push_substring(seg.payload().copy(), abs_seqno - 1, eof);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_syn) {
        return wrap(_reassembler.expected_index() + 1 + (_reassembler.stream_out().input_ended() && _fin ? 1 : 0),
                    _isn.value());
    }
    return nullopt;
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
