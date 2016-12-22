.section .rodata

    .align 2
testm_palette_pal:
    dc.w    0x0000, 0x0A04, 0x0008, 0x044E, 0x0EEE, 0xF08D, 0x7F9C, 0x0000
    dc.w    0x0F90, 0x01AB, 0x0000, 0x0000, 0x0E41, 0x0000, 0x0000, 0x0000

    .align 2
testm_palette:
    dc.w    0, 16
    dc.l    testm_palette_pal

    .align 2
testm_animation0_frame0_sprite0_tileset_tiles:
    dc.w    0x0222, 0x2220, 0x2221, 0x1222, 0x0222, 0x2220, 0x2222, 0x2222
    dc.w    0x2233, 0x3322, 0x0242, 0x2420, 0x2222, 0x2222, 0x0222, 0x2220

    .align 2
testm_animation0_frame0_sprite0_tileset:
    dc.w    0
    dc.w    1
    dc.l    testm_animation0_frame0_sprite0_tileset_tiles

    .align 2
testm_animation0_frame0_sprite0:
    dc.w    0
    dc.w    0
    dc.w    0
    dc.w    0
    dc.l    testm_animation0_frame0_sprite0_tileset


    .align 2
testm_animation0_frame0_sprites:
    dc.l    testm_animation0_frame0_sprite0

    .align 2
testm_animation0_frame0:
    dc.w    1
    dc.l    testm_animation0_frame0_sprites
    dc.w    0
    dc.l    0
    dc.w    8
    dc.w    8
    dc.w    0


    .align 2
testm_animation0_frames:
    dc.l    testm_animation0_frame0

    .align 2
testm_animation0_sequence:
    dc.w    0x009B

    .align 2
testm_animation0:
    dc.w    1
    dc.l    testm_animation0_frames
    dc.w    1
    dc.l    testm_animation0_sequence
    dc.w    0


    .align 2
testm_animations:
    dc.l    testm_animation0

    .align 2
    .global testm
testm:
    dc.l    testm_palette
    dc.w    1
    dc.l    testm_animations

