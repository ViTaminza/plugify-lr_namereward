{
    global:
        Plugify_*;

        /* s2sdk binding slots (defined by s2sdk_bindings, filled by the plugify module) */
        __s2sdk_*;

        /* levels_ranks binding slots (defined by the generated levels_ranks.cpp, filled by the
           plugify module from the levels_ranks plugin's exported methods) */
        __levels_ranks_*;

        /* hudmenu binding slots (defined by hudmenu_bindings, PUBLIC-linked through pisexcompat) */
        __hudmenu_*;

        /* No exported methods: lr_namereward exposes nothing to other plugins. */

    local: *;
};
