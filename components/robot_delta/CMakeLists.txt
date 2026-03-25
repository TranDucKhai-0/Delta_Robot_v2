idf_component_register(SRCS "src/robot_delta.c"
                    INCLUDE_DIRS "include"
                    REQUIRES type_data   
                    PRIV_REQUIRES kinematics math_until)