zephyr_library()
zephyr_library_sources_ifdef(CONFIG_FRAMEWORK sys_core.c sys_msg.c sys_cfg.c sys_shell.c buffer_pool.c)
zephyr_library_sources_ifdef(CONFIG_BUFFER_POOL_SHELL buffer_shell.c)
