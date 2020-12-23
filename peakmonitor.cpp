/*  Author:     Zoya Samsonov
 *  Created:    December 23, 2020
 */

#include <iostream>
#include <cstring>
#include "peakmonitor.h"
#include "config.h"

PeakMonitor::PeakMonitor(std::string sink_name, uint32_t rate) :
    sink_name(sink_name),
    rate(rate)
{
    pa_mainloop_init();
}

PeakMonitor::PeakMonitor(uint32_t index, uint32_t rate) :
    sink_index(index),
    rate(rate)
{
    pa_mainloop_init();
}

void PeakMonitor::pa_mainloop_init() {
    pa_threaded_mainloop* mainloop = pa_threaded_mainloop_new();
    pa_mainloop_api* api = pa_threaded_mainloop_get_api(mainloop);
    pa_context* context = pa_context_new(api, "peak_demo");
    pa_context_set_state_callback(context, 
            context_notify_cb, this);
    pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);
    pa_threaded_mainloop_start(mainloop);
}

void PeakMonitor::context_notify_cb(pa_context *c, void* userdata) {
    pa_context_state_t state = pa_context_get_state(c);
    if (state == PA_CONTEXT_READY) {
        if (!HIDE_PREAMBLE)
            std::cout << "Pulseaudio connection ready...\n\r";
        pa_operation* o = pa_context_get_sink_info_list(
                c, sink_info_cb, userdata);
        pa_operation_unref(o);
    } else if (state == PA_CONTEXT_FAILED) {
        std::cout << "Connection failed!\n\r";
    } else if (state == PA_CONTEXT_TERMINATED) {
        std::cout << "Connection terminated!\n\r";
    }
}

void PeakMonitor::sink_info_cb(pa_context *c, const pa_sink_info *i, 
        int eol, void* userdata) {
    PeakMonitor* pm = (PeakMonitor*)userdata;
    if (i == NULL)
        return;

    if (!HIDE_PREAMBLE) {
        char temp[61];
        memset(temp, '-', 60);
        temp[60] = '\0';
        std::cout << temp << "\n\r";
        std::cout << "index: " << i->index << "\n\r";
        std::cout << "name:  " << i->name << "\n\r";
        std::cout << "description: " << i->description << "\n\r";
    }

    if (!pm->sink_name.compare(i->name) || pm->sink_index == i->index) { // if i->name == sink_name
        if (!HIDE_PREAMBLE) {
            std::cout << "\n\rSetting up peak recording using ";
            std::cout << i->monitor_source_name << "\n\r\n\r";
            std::cout << "(Press q to exit at any time)\n\r\n\r";
        }

        pa_sample_spec spec = pa_sample_spec{
            .format = PA_SAMPLE_U8,
            .rate = pm->rate,
            .channels = 1
        };

        pa_stream* stream = pa_stream_new(
                c, "Peak Detect Demo", &spec, nullptr);
        pa_stream_set_read_callback(
                stream, stream_rq_cb, userdata);
        pa_stream_connect_record(
                stream, i->monitor_source_name, nullptr, 
                PA_STREAM_PEAK_DETECT);
    }
}

void PeakMonitor::stream_rq_cb(pa_stream* stream, size_t nbytes, 
        void* userdata) {
    PeakMonitor* pm = (PeakMonitor*)userdata;
    const uint8_t* data;
    pa_stream_peek(stream, (const void**)&data, &nbytes);
    for (auto i = 0UL; i < nbytes; i++) {
        int d = (int)(data[i]-128);
        pm->q.push(d);
    }
    pa_stream_drop(stream);
}
