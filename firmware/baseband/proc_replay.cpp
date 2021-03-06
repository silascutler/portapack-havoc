/*
 * Copyright (C) 2016 Jared Boone, ShareBrained Technology, Inc.
 * Copyright (C) 2016 Furrtek
 *
 * This file is part of PortaPack.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "proc_replay.hpp"

//#include "dsp_fir_taps.hpp"

#include "event_m4.hpp"

#include "utility.hpp"

ReplayProcessor::ReplayProcessor() {

}

void ReplayProcessor::execute(const buffer_c8_t& buffer) {
	/* 2.4576MHz, 2048 samples */
	//const auto decim_0_out = decim_0.execute(buffer, dst_buffer);
	//const auto decim_1_out = decim_1.execute(decim_0_out, dst_buffer);
	//const auto& decimator_out = decim_1_out;
	//const auto& channel = decimator_out;

	//if( stream ) {
	//	const size_t bytes_to_write = sizeof(*decimator_out.p) * decimator_out.count;
	//	const auto result = stream->write(decimator_out.p, bytes_to_write);
	//}

	//feed_channel_stats(channel);

	/*spectrum_samples += channel.count;
	if( spectrum_samples >= spectrum_interval_samples ) {
		spectrum_samples -= spectrum_interval_samples;
		channel_spectrum.feed(channel, channel_filter_pass_f, channel_filter_stop_f);
	}*/
}

void ReplayProcessor::on_message(const Message* const message) {
	switch(message->id) {
	/*case Message::ID::UpdateSpectrum:
	case Message::ID::SpectrumStreamingConfig:
		channel_spectrum.on_message(message);
		break;*/

	case Message::ID::ReplayConfig:
		replay_config(*reinterpret_cast<const ReplayConfigMessage*>(message));
		break;

	default:
		break;
	}
}

void ReplayProcessor::replay_config(const ReplayConfigMessage& message) {
	if( message.config ) {
		stream = std::make_unique<StreamOutput>(message.config);
	} else {
		stream.reset();
	}
}

int main() {
	EventDispatcher event_dispatcher { std::make_unique<ReplayProcessor>() };
	event_dispatcher.run();
	return 0;
}
