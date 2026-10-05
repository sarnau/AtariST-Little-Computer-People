# Port names -> the Ghidra analysis names

The analysis documents in this folder (ARCHITECTURE.md, PEOPLE.md, DOG.md,
GAMES.md, SOUND.md, IMAGEFORMAT.md) use the descriptive names of the
first Ghidra analysis, such as `action_brush_teeth`.  This table maps the
port's names to them.

The Ghidra project itself now carries the port's names (re-synced
2026-10-05), so the right-hand column is for reading those documents;
searching Ghidra for it will not find anything.

## Functions

| short   | long                                             |
|---------|--------------------------------------------------|
| brushTeeth | action_brush_teeth                               |
| crouchForPat | action_call_dog                                  |
| checkFrontDoor | action_check_front_door                          |
| cleanUp | action_clean_up                                  |
| closeBedCloset | action_close_closet_door                         |
| closeToiletDoor | action_close_toilet_door                         |
| danceToMusic | action_dance                                     |
| drinkWater | action_drink                                     |
| washAtSink | action_drink_water_animation                     |
| cookMeal  | action_eat_meal                                  |
| feedDog | action_feed_dog                                  |
| nodOk  | action_get_dressed                               |
| getInOutOfBed | action_get_in_out_of_bed                         |
| goToFridge | action_get_snack_from_fridge                     |
| nightRoutine | action_go_to_bed_night                           |
| sayHello | action_hello                                     |
| eatFromCabinet | action_kitchen_cabinet                           |
| lightFire | action_light_fireplace                           |
| playRecord | action_listen_song                               |
| nodHead  | action_nod_head                                  |
| changeClothes | action_open_close_bedroom_closet                 |
| openKitchenCab | action_open_close_cabinet                        |
| openDresser | action_open_close_dresser                        |
| closeFilingCab | action_open_close_filing_cabinet                 |
| putInFridge | action_open_close_fridge                         |
| openFrontDoor | action_open_close_front_door                     |
| enterStudy | action_open_close_upstairs_closet                |
| paceNervously | action_pace_nervously                            |
| peekAround | action_peek_around                               |
| waitForPat  | action_pet_dog                                   |
| playGame | action_play_a_game                               |
| useComputer | action_play_computer                             |
| stopRecord | action_play_piano                                |
| playOrgan | action_play_with_record                          |
| readNewspaper | action_read_newspaper                            |
| exercise | action_sit_and_exercise                          |
| readInArmchair | action_sit_on_couch_with_dog                     |
| dozeOff | action_sleep                                     |
| takeShower | action_take_shower                               |
| tidyHouse | action_tidy_house                                |
| toggleTv | action_toggle_tv                                 |
| useToilet  | action_use_toilet                                |
| wakeFromAlarm | action_wake_from_alarm                           |
| morningRoutine | action_wake_up_morning                           |
| rummageCabinet | action_walk_to_and_turn                          |
| idleShrug | action_wander_idly                               |
| washHands | action_wash_hands                                |
| writeLetter | action_write_letter                              |
| yawnAndStretch | action_yawn_and_stretch                          |
| al_locs | asset_load_character_sheets                      |
| loadFrameFile | asset_load_lcp                                   |
| ~~al_loan~~ | asset_load_names -- DELETED, LCP_STX has no such    |
|         | function; lcp_crnd inlines the NAMES read           |
| al_loot | asset_load_objects_table                         |
| loadSprites   | asset_load_sprites_table (was al_lost)           |
| bookDelivery | event_receive_book_delivery                      |
| dogFoodDelivery | event_receive_dog_food                           |
| foodDelivery | event_receive_food_delivery                      |
| recordDelivery | event_receive_record_delivery                    |
| readFile | file_read                                        |
| unpackFile | file_read_compressed                             |
| tvStoop | lcp_idle_look_left                               |
| recordStoop | lcp_idle_look_right                              |
| typeKeySound | letter_select_typewriter_sound                   |
| typeChar | letter_type_character_animated                   |
| typeString | letter_type_string_animated                      |
| mh_chac | midi_header_handle_channel_count                 |
| mh_proc | midi_header_handle_program_change                |
| mh_scat | midi_header_handle_scale_table                   |
| mh_temp | midi_header_handle_tempo                         |
| mh_volu | midi_header_handle_volume                        |
| buildNoteMap | midi_seq_build_scale_table                       |
| sendMidiEvent | midi_seq_dispatch_event                          |
| startSong | midi_seq_init_song                               |
| unpackChanMap | midi_seq_parse_channel_map                       |
| parseSongHeader | midi_seq_parse_header                            |
| resetPrograms | midi_seq_reset_programs                          |
| sendProgChange | midi_seq_send_program_change                     |
| initSongState | midi_seq_set_position                            |
| skipTextField | midi_seq_skip_padding                            |
| armSequencer | midi_seq_start_playback                          |
| pickClothes | palette_apply_clothing_colors                    |
| pickSkin | palette_apply_skin_colors                        |
| drawFoodCab | screen_draw_food_cabinet                         |
| blackRow | screen_fill_row_black                            |
| stripeRow | screen_fill_row_striped                          |
| paperRow | screen_fill_row_white                            |
| renderFrame | screen_render_8hz                                |
| scrollStrip | screen_scroll_text_down                          |
| beginDraw | screen_set_draw_to_backbuffer                    |
| endDraw | screen_set_draw_to_frontbuffer                   |
| startSfx | soundeffect_irq_play                             |
| sfxSelect | soundeffect_select                               |
| loadSounds   | soundeffects_load                                |
| stopSfx   | soundeffects_off                                 |
| drawSlot | sprite_draw                                      |
| flipSprite | sprite_flip_horizontal                           |
| initMfdb | sprite_init_MFDB                                 |
| expandFrame | sprite_lcp_flip                                  |
| stepHead | sprite_lcp_head_animate                          |
| updateHead | sprite_lcp_head_update                           |
| updateBody | sprite_update_body                               |
| layoutSlots | sprite_update_slots                              |
| activateSprite | spritedata_select                                |
| carryBehind | spritedata_select_carried_object_left            |
| carryInFront | spritedata_select_carried_object_right           |
| setDogSprite | spritedata_update_dog                            |
| drawTvPicture | tv_draw_static_line                              |
| tvNoise | tv_draw_static_noise                             |
| tvBounce | tv_show_bouncing_line                            |
| tvPattern | tv_show_pattern_lines                            |
| tvClearAnim | tv_show_screen_clear                             |
| tvOff  | tv_turn_off                                      |
| tvOn   | tv_turn_on                                       |

## Globals

| short   | long                                             |
|---------|--------------------------------------------------|
| queueCount | _action_list_size                                |
| cmdPriority | _action_priority                                 |
| queuePriority | _action_priority_queue                           |
| queueActions | _action_queue                                    |
| g_hzhi  | _hz_200_hi                                       |
| g_hzlo  | _hz_200_lo                                       |
| anaExtraGuess | anagram_all_clues_used                           |
| anaNumClues | anagram_clue_count                               |
| anaGuessNum | anagram_guess_number                             |
| anaInput | anagram_input_buffer                             |
| anaAnswer | anagram_original_word                            |
| anaScrambled | anagram_scrambled_word                           |
| anaWordLen | anagram_word_length                              |
| clockHour | clock_hour                                       |
| hourHandXY | clock_hour_position                              |
| clockMinute | clock_minute                                     |
| minuteHandXY | clock_minute_position                            |
| shirtPrimary | clothing_color_primary                           |
| shirtSecondary | clothing_color_secondary                         |
| typedLine | command_input_buffer                             |
| typedCursor | command_input_buffer_pos                         |
| dogXNudge | dog_dest_x_offset_table                          |
| dogYNudge | dog_dest_y_offset_table                          |
| dogRoamSpots | dog_destination_position_table                   |
| dogEating | dog_eating_active                                |
| dogEatCount | dog_eating_countdown                             |
| dogMirImage | dog_flip_image_buffer                            |
| dogMirMask | dog_flip_mask_buffer                             |
| dogEatFrames | dog_sprite_eating_anim_tab                       |
| dogSpriteId  | dog_sprite_id                                    |
| dogXTarget   | dog_target_x                                     |
| dogYTarget   | dog_target_y                                     |
| dogStepIdx | dog_walk_anim_cycle                              |
| dogWalkSprites | dog_walk_anim_frames                             |
| dogXWaypt   | dog_waypoint_x                                   |
| dogYWaypt   | dog_waypoint_y                                   |
| headPose | head_anim_current                                |
| headDelay | head_anim_delay_countdown                        |
| headMode | head_anim_mode                                   |
| headLastWalk | head_anim_state_last                             |
| headTarget | head_anim_target_state                           |
| headImage | head_sprite_buffer                               |
| headFrame | head_sprite_frame                                |
| headMask | head_sprite_mask                                 |
| headMirror | head_sprite_mirror_flag                          |
| carriedSprite | lcp_carried_object                               |
| isCarrying | lcp_carrying_object_flag                         |
| bodyImage | lcp_sprite_img                                   |
| bodyMask | lcp_sprite_mask                                  |
| lcpHidden  | lcp_sprites_hidden                               |
| typingSprites | letter_char_width_table                          |
| needlePos | letter_line_count                                |
| vuLeds | letter_paragraph_count                           |
| letterWord | letter_scratch_buffer                            |
| g_mccha | midi_current_channel                             |
| sentProgram | midi_current_program                             |
| g_mnevc | midi_note_event_count                            |
| g_mnevi | midi_note_event_index                            |
| g_mnhil | midi_note_hi_limit                               |
| g_mnlol | midi_note_lo_limit                               |
| g_msmap | midi_seq_max_position                            |
| seqPhase | midi_seq_phase                                   |
| songActive | midi_sequencer_active                            |
| g_molof | midi_song_loop_flag                              |
| songMaxPos | midi_song_max_position                           |
| timerTicks | midi_tick_counter                                |
| envDivider | midi_tick_divider                                |
| seqCountdown | midi_tick_prescaler                              |
| ticksPerBeat | midi_ticks_per_beat                              |
| g_obibg | object_id_blue_green                             |
| g_obicc | object_id_cabinet_closed                         |
| g_obico | object_id_cabinet_open_1                         |
| g_obi02 | object_id_cabinet_open_2                         |
| g_obidc | object_id_door_closet_closed                     |
| g_obi03 | object_id_door_closet_open_1                     |
| g_obi04 | object_id_door_closet_open_2                     |
| g_obidf | object_id_door_front_closed                      |
| g_obi05 | object_id_door_front_open_1                      |
| g_obi06 | object_id_door_front_open_2                      |
| g_obids | object_id_door_study_closed                      |
| g_obi07 | object_id_door_study_open_1                      |
| g_obi08 | object_id_door_study_open_2                      |
| g_obidt | object_id_door_toilet_closed                     |
| g_obi09 | object_id_door_toilet_open_1                     |
| g_obi10 | object_id_door_toilet_open_2                     |
| g_obi11 | object_id_dresser_closed                         |
| g_obido | object_id_dresser_open_1                         |
| g_obi12 | object_id_dresser_open_2                         |
| g_obifc | object_id_filing_cabinet_closed                  |
| g_obi13 | object_id_filing_cabinet_open_1                  |
| g_obi14 | object_id_filing_cabinet_open_2                  |
| g_obifa | object_id_fireplace_animation                    |
| g_obifo | object_id_fireplace_off                          |
| g_obi15 | object_id_fridge_closed                          |
| g_obi16 | object_id_fridge_open_1                          |
| g_obi17 | object_id_fridge_open_2                          |
| g_obipc | object_id_phone_call                             |
| stoveFrames | object_id_stove_animation                        |
| g_obiso | object_id_stove_off                              |
| objHeights | object_tab_height                                |
| objMfdbs | object_tab_mfdb_table                            |
| objWidths | object_tab_width                                 |
| patFrame | petting_anim_frame                               |
| patActive | petting_dog_active                               |
| bjBetMain | poker_computer_bet                               |
| compPile | poker_computer_draw_pile                         |
| compChips | poker_computer_money                             |
| bjBetSplit | poker_player_bet                                 |
| plyrPile | poker_player_draw_pile                           |
| plyrChips | poker_player_money                               |
| potChips | poker_pot_amount                                 |
| frameMfdb | screen_mfdb                                      |
| stripScroll | screen_scroll_down_count                         |
| sfxBuffer | soundeffect_DoSound_Buffer                       |
| sfxStartHz | soundeffect_Hz200                                |
| sfxPending | soundeffect_active_flag                          |
| sfxReqId | soundeffect_current                              |
| sfxCurPrio | soundeffect_current_priority                     |
| sfxDurHi | soundeffect_default_duration_hi                  |
| sfxDurLo | soundeffect_default_duration_lo                  |
| sfxDosCtl | soundeffect_dosound_control                      |
| sfxDosStat | soundeffect_dosound_status                       |
| sfxReqDur | soundeffect_duration                             |
| sfxPlaying | soundeffect_playing_flag                         |
| sfxCurId | soundeffect_playing_id                           |
| sfxTicksLeft | soundeffect_remaining_ticks                      |
| drawnHeight | sprite_active_height                             |
| drawnWidth | sprite_active_width                              |
| drawnX | sprite_active_x                                  |
| drawnY | sprite_active_y                                  |
| spriteHeight | sprite_def_height                                |
| spriteWidth | sprite_def_width                                 |
| spriteLayer | sprite_layer_flags                               |
| slotImgMfdb | sprite_mfdb_image                                |
| slotMaskMfdb | sprite_mfdb_mask                                 |
| pendReady | sprite_pending_flag                              |
| pendHeight | sprite_pending_height                            |
| pendWidth | sprite_pending_width                             |
| pendX | sprite_pending_x                                 |
| pendY | sprite_pending_y                                 |
| spriteSlot | sprite_slot_map                                  |
| g_setah | sprite_tab_height                                |
| g_setmt | sprite_tab_mfdb_table                            |
| g_setaw | sprite_tab_width                                 |
| tvBar0X | tv_pattern_0_x_coords                            |
| tvBar0Y | tv_pattern_0_y_coords                            |
| tvBar1X | tv_pattern_1_x_coords                            |
| tvBar1Y | tv_pattern_1_y_coords                            |
| tvBar2X | tv_pattern_2_x_coords                            |
| tvBar2Y | tv_pattern_2_y_coords                            |
| tvBar3X | tv_pattern_3_x_coords                            |
| tvBar3Y | tv_pattern_3_y_coords                            |
| tvBarColor | tv_pattern_color_indices                         |
| walkXTarget   | walk_target_x                                    |
| walkYTarget   | walk_target_y                                    |
| xWaypoint   | walk_waypoint_x                                  |
| yWaypoint   | walk_waypoint_y                                  |

## Second-pass renames (cross-namespace collisions)

These 7-char prefixes collided between the function and global
namespaces, which my initial per-kind analysis missed.

| short   | long                                             |
|---------|--------------------------------------------------|
| isDogDelivery | delivery_is_for_dog                              |
| dv_pick | delivery_pickup_at_door                          |
| loadSavedGame | lcp_load                                         |
| loadedSave | lcp_loaded                                       |
| aciaWrite  | midi_out_write_byte                              |
| midiOutOn  | midi_output_enabled                              |
| organPlaying | record_browsing_active                           |
| animRecPlayer | record_player_animate_needle                     |
| wpzIndex  | word_puzzle_current_index                        |
| wpzText  | word_puzzle_data_buffer                          |
| playWordPuzzle | word_puzzle_main                                 |

## Third-pass renames (post-Alcyon-compile collisions)

Discovered after starting the actual Alcyon build: identifiers
that my initial `globals.h` extern extraction missed, plus
multiple items sharing prefixes not previously flagged.

| short   | long                                             |
|---------|--------------------------------------------------|
| phraseBits   | _entered_word_bytes                              |
| phraseTable  | _enteredword_to_action                           |
| wordBit  | _enteredword_to_bit                              |
| noPreempt | action_interruptible_flag                        |
| activeActions | action_table_active                              |
| moderateActions | action_table_moderate                            |
| relaxedActions | action_table_relaxed                             |
| playAnagrams | anagram_main                                     |
| anaDict  | anagram_words_buffer                             |
| anaWrongMsgs | anagram_wrong_guess_messages                     |
| g_dsb   | dest_scr_buffer                                  |
| stripBuf  | dest_screenbase_ptr                              |
| letterSignoffs   | letter_greeting_table                            |
| letterLines  | letter_line_ptr                                  |
| letterText  | letter_txt_content                               |
| midiMsg  | midi_event                                       |
| g_medu  | midi_event_duration                              |
| keyScaleMask  | midi_scale_mask_table                            |
| noteMap  | midi_scale_transpose_table                       |
| drawObject | object_draw                                      |
| g_oiidx | object_index                                     |
| p_dosnd | play_door_sound                                  |
| playDoorbell | play_doorbell_sound                              |
| sfxGreeting | play_soundeffect_greeting                        |
| sfxHeadNod | play_soundeffect_head_nod                        |
| sfxSpeech | play_soundeffect_speech                          |
| sfxTvClick | play_soundeffect_tv_click                        |
| posYOffset  | room_position_height_table                       |
| posXHalf  | room_position_x_table                            |
| drawLogbase | screen_logbase                                   |
| housePtr | screen_ptr                                       |
| drawnImage | sprite_active_image                              |
| drawnMask | sprite_active_mask                               |
| spriteBitmap | sprite_def_image                                 |
| spriteMask | sprite_def_mask                                  |
| g_seid  | sprite_id                                        |
| g_seix  | sprite_index                                     |
| pendImage | sprite_pending_image                             |
| pendMask | sprite_pending_mask                              |
| g_txx   | target_x                                         |
| g_txy   | target_y                                         |
| nextAction  | trigger_action                                   |
| eventQueue  | triggered_event_list                             |

## Fourth-pass renames (extern-symbol collisions from Ghidra ports)

New Ghidra-faithful ports added identifiers that collided with pre-
existing extern names at Alcyon's 7-char C name / 8-char asm boundary.
Rename each collision pair so the linker sees distinct `.comm` blocks.

| short   | long                                             |
|---------|--------------------------------------------------|
| altScreen | SCREEN_BUFFER_A                                  |
| houseBuf | SCREEN_BUFFER_B                                  |
| bitSet32  | bitmask_32bit_or                                 |
| bitClear32 | bitmask_32bit_and                                |
| curPos  | currentPosition                                  |
| midiEvP | midiEventPtr                                     |
| midiEvS | midiEventSize                                    |
| loadLetterText | file_load_letter_template                        |
| bshdbuf | body_shape_data_buf                              |
| hshdbuf | head_shape_data_buf                              |
