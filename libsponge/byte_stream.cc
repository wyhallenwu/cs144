#include "byte_stream.hh"


// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity): buffer_(),capacity_(capacity), written_count_(0),read_count_(0), end_input_(false) { }

size_t ByteStream::write(const string &data) {
    size_t write_size = remaining_capacity() > data.size() ? data.size() : remaining_capacity();
    for(size_t count = 0; count < write_size; count++){
        buffer_.emplace_back(data.at(count));
    }
    written_count_ += write_size;
    return write_size;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t peek_len = len > buffer_.size() ? buffer_.size() : len;
    string peek_str;
    auto iter = buffer_.begin();
    for(size_t count = 0; count < peek_len; count++){
        peek_str += *iter;
        ++iter;
    }
    return peek_str;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    size_t pop_size = len > buffer_.size() ? buffer_.size() : len;
    for(size_t count = 0; count < pop_size; count++){
        buffer_.pop_front();
    }
    read_count_ += pop_size;
 }

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    std::string read_bytes = peek_output(len);
    pop_output(len);
    return read_bytes;
}

void ByteStream::end_input() {end_input_ = true;}

bool ByteStream::input_ended() const { return end_input_; }

size_t ByteStream::buffer_size() const { return buffer_.size(); }

bool ByteStream::buffer_empty() const { return buffer_.empty(); }

bool ByteStream::eof() const { 
    return end_input_ && buffer_.empty();
}

size_t ByteStream::bytes_written() const { return written_count_; }

size_t ByteStream::bytes_read() const { return read_count_; }

size_t ByteStream::remaining_capacity() const { return capacity_ - buffer_.size(); } 
