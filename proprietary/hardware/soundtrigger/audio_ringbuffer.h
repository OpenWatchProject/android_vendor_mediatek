#ifndef AUDIO_RINGBUFFER_H
#define AUDIO_RINGBUFFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



#define UPPER_BOUND(in,up)      ((in) > (up) ? (up) : (in))
#define LOWER_BOUND(in,lo)      ((in) < (lo) ? (lo) : (in))
#define BOUNDED(in,up,lo)       ((in) > (up) ? (up) : (in) < (lo) ? (lo) : (in))
#define MAXIMUM(a,b)            ((a) > (b) ? (a) : (b))
#define MINIMUM(a,b)            ((a) < (b) ? (a) : (b))

struct ring_buffer_information {
    uint32_t write_pointer;
    uint32_t read_pointer;
    uint32_t buffer_byte_count;
    uint8_t *buffer_base_pointer;
};

/*
Function Name:
    ring_buffer_get_data_byte_count
Return Type:
    uint32_t
Parameters:
    struct ring_buffer_information *p_info
Function Content with Full Assertion Protection:
    uint32_t buffer_byte_count;
    uint32_t write_pointer;
    uint32_t read_pointer;
    uint32_t data_byte_count;
    // ASSERT_IF_FALSE(p_info != NULL);
    buffer_byte_count = p_info->buffer_byte_count;  // ASSERT_IF_FALSE(buffer_byte_count < 0x80000000);
    write_pointer     = p_info->write_pointer;      // ASSERT_IF_FALSE(write_pointer < (buffer_byte_count << 1));
    read_pointer      = p_info->read_pointer;       // ASSERT_IF_FALSE(read_pointer < (buffer_byte_count << 1));
    if (write_pointer >= read_pointer) {
        data_byte_count = write_pointer - read_pointer;
    } else { // write_pointer < read_pointer
        data_byte_count = (buffer_byte_count << 1) - read_pointer + write_pointer;
    }
    // ASSERT_IF_FALSE(data_byte_count <= buffer_byte_count);
    return data_byte_count;
Function Name:
    ring_buffer_get_space_byte_count
Return Type:
    uint32_t
Parameters:
    struct ring_buffer_information *p_info
Function Content with Full Assertion Protection:
    uint32_t buffer_byte_count;
    uint32_t data_byte_count;
    uint32_t space_byte_count;
    // ASSERT_IF_FALSE(p_info != NULL);
    buffer_byte_count = p_info->buffer_byte_count;                  // ASSERT_IF_FALSE(buffer_byte_count < 0x80000000);
    data_byte_count   = ring_buffer_get_data_byte_count(p_info);    // ASSERT_IF_FALSE(data_byte_count <= buffer_byte_count);
    space_byte_count  = buffer_byte_count - data_byte_count;
    return space_byte_count;
Function Name:
    ring_buffer_get_write_information
Return Type:
    void
Parameters:
    struct ring_buffer_information *p_info
    uint8_t **pp_buffer
    uint32_t *p_byte_count
Function Content with Full Assertion Protection:
    uint32_t buffer_byte_count;
    uint32_t space_byte_count;
    uint8_t *buffer_pointer;
    uint32_t write_pointer;
    uint32_t read_pointer;
    uint32_t tail_byte_count;
    uint32_t write_byte_cnt;
    // ASSERT_IF_FALSE(p_info != NULL);
    // ASSERT_IF_FALSE(pp_buffer != NULL);
    // ASSERT_IF_FALSE(p_byte_count != NULL);
    buffer_byte_count = p_info->buffer_byte_count;                  // ASSERT_IF_FALSE(buffer_byte_count < 0x80000000);
    space_byte_count  = ring_buffer_get_space_byte_count(p_info);   // ASSERT_IF_FALSE(space_byte_count <= buffer_byte_count);
    buffer_pointer    = p_info->buffer_base_pointer;                // ASSERT_IF_FALSE(buffer_pointer != NULL);
    write_pointer     = p_info->write_pointer;                      // ASSERT_IF_FALSE(write_pointer < (buffer_byte_count << 1));
    read_pointer      = p_info->read_pointer;                       // ASSERT_IF_FALSE(read_pointer < (buffer_byte_count << 1));
    if (write_pointer < buffer_byte_count) {    // 1st mirror part
        buffer_pointer += write_pointer;
        tail_byte_count = buffer_byte_count - write_pointer;
    } else {                                    // 2nd mirror part
        buffer_pointer += write_pointer - buffer_byte_count;
        tail_byte_count = (buffer_byte_count << 1) - write_pointer;
    }
    write_byte_cnt = MINIMUM(data_byte_count, tail_byte_count);
    *pp_buffer = buffer_pointer;
    *p_byte_count = write_byte_cnt;
    return;
Function Name:
    ring_buffer_get_read_information
Return Type:
    void
Parameters:
    struct ring_buffer_information *p_info
    uint8_t **pp_buffer
    uint32_t *p_byte_count
Function Content with Full Assertion Protection:
    uint32_t buffer_byte_count;
    uint32_t data_byte_count;
    uint8_t *buffer_pointer;
    uint32_t write_pointer;
    uint32_t read_pointer;
    uint32_t tail_byte_count;
    uint32_t read_byte_cnt;
    // ASSERT_IF_FALSE(p_info != NULL);
    // ASSERT_IF_FALSE(pp_buffer != NULL);
    // ASSERT_IF_FALSE(p_byte_count != NULL);
    buffer_byte_count = p_info->buffer_byte_count;                  // ASSERT_IF_FALSE(buffer_byte_count < 0x80000000);
    data_byte_count   = ring_buffer_get_data_byte_count(p_info);    // ASSERT_IF_FALSE(data_byte_count <= buffer_byte_count);
    buffer_pointer    = p_info->buffer_base_pointer;                // ASSERT_IF_FALSE(buffer_pointer != NULL);
    write_pointer     = p_info->write_pointer;                      // ASSERT_IF_FALSE(write_pointer < (buffer_byte_count << 1));
    read_pointer      = p_info->read_pointer;                       // ASSERT_IF_FALSE(read_pointer < (buffer_byte_count << 1));
    if (read_pointer < buffer_byte_count) { // 1st mirror part
        buffer_pointer += read_pointer;
        tail_byte_count = buffer_byte_count - read_pointer;
    } else {                                // 2nd mirror part
        buffer_pointer += read_pointer - buffer_byte_count;
        tail_byte_count = (buffer_byte_count << 1) - read_pointer;
    }
    read_byte_cnt = MINIMUM(data_byte_count, tail_byte_count);
    *pp_buffer = buffer_pointer;
    *p_byte_count = read_byte_cnt;
    return;
Function Name:
    ring_buffer_write_done
Return Type:
    void
Parameters:
    struct ring_buffer_information *p_info
    uint32_t write_byte_count
Function Content with Full Assertion Protection:
    uint32_t buffer_byte_count;
    uint32_t buffer_end;
    // uint32_t space_byte_count;
    uint32_t write_pointer;
    // uint32_t tail_byte_count
    // ASSERT_IF_FALSE(p_info != NULL);
    buffer_byte_count = p_info->buffer_byte_count;                  // ASSERT_IF_FALSE(buffer_byte_count < 0x80000000);
    buffer_end        = buffer_byte_count << 1;
    // space_byte_count  = ring_buffer_get_space_byte_count(p_info);   // ASSERT_IF_FALSE(space_byte_count <= buffer_byte_count);
    //                                                                 // ASSERT_IF_FALSE(write_byte_count <= space_byte_count);
    write_pointer = p_info->write_pointer; // ASSERT_IF_FALSE(write_pointer < buffer_end);
    // if (write_pointer < buffer_byte_count) { // 1st mirror part
    //     tail_byte_count = buffer_byte_count - write_pointer;
    // } else {                                // 2nd mirror part
    //     tail_byte_count = buffer_end - write_pointer;
    // }
    // ASSERT_IF_FALSE(write_byte_count <= tail_byte_count);
    write_pointer += write_byte_count;
    if (write_pointer >= buffer_end) {
        write_pointer -= buffer_end;
    }
    p_info->write_pointer = write_pointer;
    return;
Function Name:
    ring_buffer_read_done
Return Type:
    void
Parameters:
    struct ring_buffer_information *p_info
    uint32_t read_byte_count
Function Content with Full Assertion Protection:
    uint32_t buffer_byte_count;
    uint32_t buffer_end;
    // uint32_t data_byte_count;
    uint32_t read_pointer;
    // uint32_t tail_byte_count
    // ASSERT_IF_FALSE(p_info != NULL);
    buffer_byte_count = p_info->buffer_byte_count;                  // ASSERT_IF_FALSE(buffer_byte_count < 0x80000000);
    buffer_end        = buffer_byte_count << 1;
    // data_byte_count   = ring_buffer_get_data_byte_count(p_info);    // ASSERT_IF_FALSE(data_byte_count <= buffer_byte_count);
    //                                                                 // ASSERT_IF_FALSE(read_byte_count <= data_byte_count);
    read_pointer = p_info->read_pointer; // ASSERT_IF_FALSE(read_pointer < buffer_end);
    // if (read_pointer < buffer_byte_count) { // 1st mirror part
    //     tail_byte_count = buffer_byte_count - read_pointer;
    // } else {                                // 2nd mirror part
    //     tail_byte_count = buffer_end - read_pointer;
    // }
    // ASSERT_IF_FALSE(read_byte_count <= tail_byte_count);
    read_pointer += read_byte_count;
    if (read_pointer >= buffer_end) {
        read_pointer -= buffer_end;
    }
    p_info->read_pointer = read_pointer;
    return;
*/

uint32_t ring_buffer_get_data_byte_count(struct ring_buffer_information *p_info);
uint32_t ring_buffer_get_space_byte_count(struct ring_buffer_information *p_info);
void ring_buffer_get_write_information(struct ring_buffer_information *p_info, uint8_t **pp_buffer, uint32_t *p_byte_count);
void ring_buffer_get_read_information(struct ring_buffer_information *p_info, uint8_t **pp_buffer, uint32_t *p_byte_count);
void ring_buffer_write_done(struct ring_buffer_information *p_info, uint32_t write_byte_count);
void ring_buffer_read_done(struct ring_buffer_information *p_info, uint32_t read_byte_count);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of AUDIO_RINGBUFFER_H */

