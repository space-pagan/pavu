/*  Author:     Zoya Samsonov
 *  Created:    December 23, 2020
 */

#include <iostream>
#include <cstring>
#include "config.h"
#include "monitor.h"

// instantiating with a stream name means we can always find the correct
// stream (if it exists)
Monitor::Monitor(std::string stream_name, uint32_t rate) :
    stream_name(stream_name),
    rate(rate)
{
    pa_mainloop_init();
}

// instantiating with a stream index requires us to know if it is a sink or 
// a source, since there may be a sink and a source with the same index
Monitor::Monitor(uint32_t index, bool is_sink, uint32_t rate) :
    stream_index(index),
    rate(rate),
    is_sink(is_sink)
{
    pa_mainloop_init();
}

// create a pa_context, set the callback function to context_notify_cb
// with userdata being this instance of the Monitor class, start the
// threaded mainloop
void Monitor::pa_mainloop_init() {
    pa_threaded_mainloop* mainloop = pa_threaded_mainloop_new();
    pa_mainloop_api* api = pa_threaded_mainloop_get_api(mainloop);
    pa_context* context = pa_context_new(api, "peak_demo");
    pa_context_set_state_callback(context, 
            context_notify_cb, this);
    pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);
    pa_threaded_mainloop_start(mainloop);
}

// gets called once the pa_context is set
void Monitor::context_notify_cb(pa_context *c, void* userdata) {
    pa_context_state_t state = pa_context_get_state(c);
    if (state == PA_CONTEXT_READY) {
        // only print this stuff if the user has set the setting to do so
        if (!HIDE_PREAMBLE)
            std::cout << "Pulseaudio connection ready...\n\r"
                      << "Reading available sinks...\n\r";
        // calls sink_info_cb for each sink
        pa_operation* o = pa_context_get_sink_info_list(
                c, sink_info_cb, userdata);
        pa_operation_unref(o);
        if (!HIDE_PREAMBLE)
            std::cout << "Reading available sources...\n\r";
        // calls source_info_cb for each source
        pa_operation* p = pa_context_get_source_info_list(
                c, source_info_cb, userdata);
        pa_operation_unref(p);
    } else if (state == PA_CONTEXT_FAILED) {
        // something went wrong
        std::cerr << "Connection failed!\n\r";
    } else if (state == PA_CONTEXT_TERMINATED) {
        // the context was terminated
        std::cerr << "Connection terminated!\n\r";
    }
}

// just a wrapper for sink_source_info_cb
void Monitor::sink_info_cb(pa_context *c, const pa_sink_info *i, 
        int eol, void* userdata) {
    sink_source_info_cb(c, i, nullptr, userdata);
}

// just a wrapper for sink_source_info_cb
void Monitor::source_info_cb(pa_context *c, const pa_source_info *i, 
        int eol, void* userdata) {
    sink_source_info_cb(c, nullptr, i, userdata);
}

// process a sink or source, check if it is the one we are searching for, etc
void Monitor::sink_source_info_cb(pa_context *c, const pa_sink_info *i,
        const pa_source_info *j, void* userdata) {
    Monitor* pm = (Monitor*)userdata;
    // both i and j are NULL, or we have already found the target stream
    if ((i == NULL && j == NULL) || pm->is_connected)
        return;

    // temporary variables so one set of code works for both sinks and sources
    const char* name = (i == NULL) ? j->name : i->name;
    const char* monitor_name = (i == NULL) ? j->name : i->monitor_source_name;
    uint32_t index = -1;
    if (i != NULL && pm->is_sink) index = i->index;
    if (j != NULL && !pm->is_sink) index = j->index;

    // if the name or the index matches, proceed
    if (!pm->stream_name.compare(name) || index == pm->stream_index) {
        if (!HIDE_PREAMBLE)
            std::cout << "Setting up peak recording using "
                      << monitor_name << "\n\r\n\r";

        // sample spec object, passed to the created stream
        pa_sample_spec spec = pa_sample_spec{
            .format = PA_SAMPLE_U8,
            .rate = pm->rate,
            .channels = 1
        };
        // create a new stream to spec
        pa_stream* stream = pa_stream_new(
                c, "PulseAudio VU Meter", &spec, nullptr);
        // set the stream callback function to stream_rq_cb
        pa_stream_set_read_callback(
                stream, stream_rq_cb, userdata);
        // connect the stream to the correct stream
        pa_stream_connect_record(
                stream, monitor_name, nullptr, 
                PA_STREAM_PEAK_DETECT);
        // do not process any further sinks/sources
        pm->is_connected = true;
    }
}

void Monitor::stream_rq_cb(pa_stream* stream, size_t nbytes, void* userdata) {
    Monitor* pm = (Monitor*)userdata;
    const uint8_t* data;
    // get new data from the stream
    pa_stream_peek(stream, (const void**)&data, &nbytes);
    // read each byte available, put it into our queue after normalizing
    for (auto i = 0UL; i < nbytes; i++) {
        int d = (int)(data[i]-128);
        pm->q.push(d);
    }
    // not sure tbh
    pa_stream_drop(stream);
}
