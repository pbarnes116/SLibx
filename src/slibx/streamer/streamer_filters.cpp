/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../../../inc/slibx/streamer/filters.h"

#include <slib/core/io.h>
#include <slib/core/scoped.h>
#include <slib/crypto/sha2.h>

namespace slib
{
	
	namespace streamer
	{
		
		List<Packet> DatagramHashSHA256SendFilter::filter(const Packet& input)
		{
			Packet output = input;
			MemoryBuffer buf;
			buf.add(SHA256::hash(input.data));
			buf.add(input.data);
			output.data = buf.merge();
			return List<Packet>::createFromElement(output);
		}
		
		List<Packet> DatagramHashSHA256ReceiveFilter::filter(const Packet& input)
		{
			Packet output = input;
			sl_uint8* buf = (sl_uint8*)(output.data.getData());
			sl_uint32 size = (sl_uint32)(output.data.getSize());
			if (size <= 32) {
				return List<Packet>::null();
			}
			sl_uint8 hash[32];
			SHA256::hash(buf + 32, size - 32, hash);
			if (Base::compareMemory(hash, buf, 32) != 0) {
				return List<Packet>::null();
			}
			output.data = Memory::create(buf + 32, size - 32);
			return List<Packet>::createFromElement(output);
		}

		DatagramErrorCorrectionSendFilter::DatagramErrorCorrectionSendFilter(sl_uint32 level, sl_uint32 maxPacketSize)
		{
			m_lastSentPacketNumber = 0;
			m_level = level;
			m_maxPacketSize = maxPacketSize;
		}
		
		DatagramErrorCorrectionSendFilter::~DatagramErrorCorrectionSendFilter()
		{
		}
		
		List<Packet> DatagramErrorCorrectionSendFilter::filter(const Packet& input)
		{
			void* buf = input.data.getData();
			sl_size size = input.data.getSize();
			if (size > m_maxPacketSize) {
				return List<Packet>::null();
			}
			SendPacket packetNew;
			packetNew.num = m_lastSentPacketNumber++;
			packetNew.mem = Memory::create(buf, size);
			m_packetsSend.push(packetNew, m_level + 1);
			MemoryWriter out;
			Array<SendPacket> packets = m_packetsSend.toArray();
			for (sl_size i = 0; i < packets.getCount(); i++) {
				out.writeUint64CVLI(packets[i].num);
				out.writeSizeCVLI(packets[i].mem.getSize());
				out.write(packets[i].mem.getData(), packets[i].mem.getSize());
			}
			Memory data = out.getData();
			
			List<Packet> ret;
			Packet output;
			output.format = Packet::formatRaw;
			output.data = data;
			ret.add(output);
			return ret;
		}
		
		DatagramErrorCorrectionReceiveFilter::DatagramErrorCorrectionReceiveFilter(sl_uint32 maxPacketSize)
		{
			m_maxPacketSize = maxPacketSize;
			m_lastReceivedPacketNumber = 0;
		}
		
		DatagramErrorCorrectionReceiveFilter::~DatagramErrorCorrectionReceiveFilter()
		{
		}
		
		List<Packet> DatagramErrorCorrectionReceiveFilter::filter(const Packet& input)
		{
			List<Packet> ret;
			void* buf = input.data.getData();
			sl_size packetSize = input.data.getSize();
			MemoryReader in(buf, packetSize);
			SLIB_SCOPED_BUFFER(char, 4096, buffer, m_maxPacketSize);
			if (!buffer) {
				return List<Packet>::null();
			}
			while (1) {
				sl_uint64 num;
				sl_size size;
				if (!in.readUint64CVLI(&num)) {
					break;
				}
				if (!in.readSizeCVLI(&size)) {
					break;
				}
				if (size == 0 || size > m_maxPacketSize) {
					break;
				}
				sl_size sizeRead = in.read(buffer, size);
				if (size == sizeRead) {
					if (num > m_lastReceivedPacketNumber || num + 100 < m_lastReceivedPacketNumber || m_lastReceivedPacketNumber == 0) {
						m_lastReceivedPacketNumber = num;
						Packet packet;
						packet.format = Packet::formatRaw;
						packet.data = Memory::create(buffer, size);
						ret.add(packet);
					}
				} else {
					break;
				}
			}
			return ret;
		}

		
	}
	
}

