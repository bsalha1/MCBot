#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include <boost/algorithm/string.hpp>

#include "Socket.h"
#include "Buffer.h"
#include "PacketEncoder.h"
#include "PacketDecoder.h"

#include "Chunk.h"
#include "Block.h"
#include "VillagerData.h"
#include "Entity.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "EntityMetadata.h"
#include "Attribute.h"
#include "Color.h"
#include "Enums.h"
#include "Slot.h"
#include "Statistic.h"
#include "UUID.h"
#include "NBT.h"
#include "Particle.h"
#include "WorldBorder.h"
#include "Position.h"
#include "Vector.h"

namespace mcbot
{
	class MCBot;
	
	class PacketReceiver
	{
	private:
		MCBot* bot;
		bool encryption_enabled;
		bool compression_enabled;

	public:
		PacketReceiver(MCBot* bot);
		PacketReceiver();

		// Server incoming requests
		void recv_packet();
		int read_next_var_int();
		int read_next_packet(int length, uint8_t* packet, int decompressed_length = 0);
		void handle_recv_packet(int packet_id, uint8_t* packet, int bytes_read, size_t& offset);

		// LOGIN
		void recv_encryption_request(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_set_compression(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_login_success(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_login_disconnect(uint8_t* packet, size_t size_read, size_t& offset);

		// PLAY
		void recv_play_disconnect(uint8_t* packet, size_t length, size_t& offset);
		void recv_keep_alive(uint8_t* packet, size_t length, size_t& offset);
		void recv_join_server(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_chat_message(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_update_time(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_equipment(uint8_t* packet, size_t length, size_t& offset);
		void recv_spawn_position(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_update_health(uint8_t* packet, size_t length, size_t& offset);
		void recv_position(uint8_t* packet, size_t length, size_t& offset);
		void recv_held_item_slot(uint8_t* packet, size_t length, size_t& offset);
		void recv_bed(uint8_t* packet, size_t length, size_t& offset);
		void recv_animation(uint8_t* packet, size_t length, size_t& offset);
		void recv_named_entity_spawn(uint8_t* packet, size_t length, size_t& offset);
		void recv_collect(uint8_t* packet, size_t length, size_t& offset);
		void recv_spawn_entity(uint8_t* packet, size_t length, size_t& offset);
		void recv_spawn_entity_living(uint8_t* packet, size_t length, size_t& offset);
		void recv_spawn_entity_painting(uint8_t* packet, size_t length, size_t& offset);
		void recv_spawn_entity_experience_orb(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_velocity(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_destroy(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity(uint8_t* packet, size_t length, size_t& offset);
		void recv_rel_entity_move(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_look(uint8_t* packet, size_t length, size_t& offset);
		void recv_rel_entity_move_look(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_teleport(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_head_look(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_status(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_metadata(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_effect(uint8_t* packet, size_t length, size_t& offset);
		void recv_experience(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_attributes(uint8_t* packet, size_t length, size_t& offset);
		void recv_map_chunk(uint8_t* packet, size_t length, size_t& offset);
		void recv_multi_block_change(uint8_t* packet, size_t length, size_t& offset);
		void recv_block_change(uint8_t* packet, size_t length, size_t& offset);
		void recv_block_break(uint8_t* packet, size_t length, size_t& offset);
		void recv_plugin_message(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_map_chunk_bulk(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_world_event(uint8_t* packet, size_t length, size_t& offset);
		void recv_named_sound_effect(uint8_t* packet, size_t length, size_t& offset);
		void recv_world_particles(uint8_t* packet, size_t length, size_t& offset);
		void recv_game_state_change(uint8_t* packet, size_t length, size_t& offset);
		void recv_set_slot(uint8_t* packet, size_t length, size_t& offset);
		void recv_window_items(uint8_t* packet, size_t length, size_t& offset);
		void recv_transaction(uint8_t* packet, size_t length, size_t& offset);
		void recv_update_sign(uint8_t* packet, size_t length, size_t& offset);
		void recv_tile_entity_data(uint8_t* packet, size_t length, size_t& offset);
		void recv_statistics(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_player_info(uint8_t* packet, size_t length, size_t& offset);
		void recv_abilities(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_scoreboard_team(uint8_t* packet, size_t length, size_t& offset);
		void recv_scoreboard_objective(uint8_t* packet, size_t length, size_t& offset);
		void recv_update_scoreboard_score(uint8_t* packet, size_t length, size_t& offset);
		void recv_display_scoreboard(uint8_t* packet, size_t length, size_t& offset);
		void recv_server_difficulty(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_world_border(uint8_t* packet, size_t length, size_t& offset);
		void recv_title(uint8_t* packet, size_t length, size_t& offset);
		void recv_player_list_header_footer(uint8_t* packet, size_t size_read, size_t& offset);
	};
}

