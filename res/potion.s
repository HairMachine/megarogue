.section .rodata

    .align 2
potion_palette_pal:
    dc.w    0x0000, 0x000E, 0x00E0, 0x00EE, 0x0E00, 0x0E0E, 0x0EE0, 0x0EEE
    dc.w    0x0CCC, 0x0AAA, 0x0888, 0x0666, 0x0444, 0x0222, 0x000C, 0x000A

    .align 2
potion_palette:
    dc.w    0, 16
    dc.l    potion_palette_pal

    .align 2
potion_animation0_frame0_sprite0_tileset_tiles:
    dc.w    0x0000, 0x0000, 0x0007, 0x7000, 0x0007, 0x7000, 0x0004, 0x4000
    dc.w    0x0046, 0x4400, 0x0044, 0x4400, 0x0004, 0x4000, 0x0000, 0x0000

    .align 2
potion_animation0_frame0_sprite0_tileset:
    dc.w    0
    dc.w    1
    dc.l    potion_animation0_frame0_sprite0_tileset_tiles

    .align 2
potion_animation0_frame0_sprite0:
    dc.w    0
    dc.w    0
    dc.w    0
    dc.w    0
    dc.l    potion_animation0_frame0_sprite0_tileset


    .align 2
potion_animation0_frame0_sprites:
    dc.l    potion_animation0_frame0_sprite0

    .align 2
potion_animation0_frame0:
    dc.w    1
    dc.l    potion_animation0_frame0_sprites
    dc.w    0
    dc.l    0
    dc.w    8
    dc.w    8
    dc.w    0


    .align 2
potion_animation0_frames:
    dc.l    potion_animation0_frame0

    .align 2
potion_animation0_sequence:
    dc.w    0x006B

    .align 2
potion_animation0:
    dc.w    1
    dc.l    potion_animation0_frames
    dc.w    1
    dc.l    potion_animation0_sequence
    dc.w    0


    .align 2
potion_animations:
    dc.l    potion_animation0

    .align 2
    .global potion
potion:
    dc.l    potion_palette
    dc.w    1
    dc.l    potion_animations

