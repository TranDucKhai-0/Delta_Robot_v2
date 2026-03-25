idf_component_register(SRCS "src/app.c"
                    INCLUDE_DIRS "include"
                    REQUIRES    
                    PRIV_REQUIRES wifi_app over_the_air globals udp_connect type_data)