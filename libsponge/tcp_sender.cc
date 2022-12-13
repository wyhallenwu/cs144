#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <optional>
#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _rto(retx_timeout)
    , _timer()
    , _capacity(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    if (!_syn) {
        // send syn seg and start timer
        TCPSegment seg;
        seg.header().syn = true;
        seg.header().seqno = _isn;
        _timer.set_timer(true);
        _segments_out.push(seg);
        _segments_outstanding.push(seg);
        _syn = true;
        _next_seqno += 1;
        _bytes_in_flight += 1;
        return;
    }
    // wrap a segment and maintain the status info
    uint16_t seg_length = max(TCPConfig::MAX_PAYLOAD_SIZE, _capacity - _bytes_in_flight);
    TCPSegment seg;
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    _receiver_window_size = window_size;
    uint64_t abs_ackno = unwrap(ackno, _isn, _next_seqno);
    // whether ack new data
    if (!ack_valid(ackno)) {
        return;
    }
    // clean up cached seg
    while (true) {
        TCPSegment &seg = _segments_outstanding.front();
        if (unwrap(seg.header().seqno + seg.length_in_sequence_space(), _isn, _next_seqno) <= abs_ackno) {
            _bytes_in_flight -= seg.length_in_sequence_space();
            _segments_outstanding.pop();
            // reset timer and related info if the ackno acks new data
            _timer.reset();
            _rto = _initial_retransmission_timeout;
            _consecutive_retransmissions_count = 0;
        } else {
            break;
        }
    }
    // fill the new window size
    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_timer.status()) {
        return;
    }
    _timer.time_pass(ms_since_last_tick);
    if (_timer.elapsed_time() >= _rto) {
        // retransmiss earliset segment
        _segments_out.push(_segments_outstanding.front());
        if (_receiver_window_size > 0 || _segments_outstanding.front().header().syn) {
            _consecutive_retransmissions_count++;
            _rto <<= 1;
        }
        _timer.reset();
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions_count; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
}
