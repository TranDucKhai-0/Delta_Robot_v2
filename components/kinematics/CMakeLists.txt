idf_component_register(SRCS "src/kinematics.c"
                    INCLUDE_DIRS "include"
                    REQUIRES globals robot_delta   
                    PRIV_REQUIRES)