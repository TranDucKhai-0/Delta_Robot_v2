import pygame
import socket
import math

# ================= CẤU HÌNH MẠNG =================
UDP_IP = "192.168.4.1" # Địa chỉ IP của ESP32 qua mạng nội bộ
UDP_PORT = 1234      # Port của ESP32
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setblocking(False) 

# ================= CẤU HÌNH VÙNG HOẠT ĐỘNG =================
R_MAX = 137.0      
Z_MIN = -350.0     
Z_MAX = -280.0     

# ================= CẤU HÌNH GIAO DIỆN =================
WIDTH, HEIGHT = 700, 700  
CENTER = (WIDTH // 2, HEIGHT // 2)
DRAW_RADIUS = int((min(WIDTH, HEIGHT) // 2) * 0.8)

pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Bảng Điều Khiển Delta Robot")
clock = pygame.time.Clock()

try:
    font = pygame.font.SysFont("tahoma, segoeui", 20)
    font_large = pygame.font.SysFont("tahoma, segoeui", 24, bold=True)
except:
    font = pygame.font.Font(pygame.font.get_default_font(), 20) 
    font_large = pygame.font.Font(pygame.font.get_default_font(), 24)

# ================= TRẠNG THÁI BAN ĐẦU =================
mode = 0           
z_step = 5.0       

# Tọa độ Mode 2 (Dùng chuột)
robot_x, robot_y, robot_z = 0.0, 0.0, -300.0   

# Tọa độ Mode 3 (Pick & Place - 2 điểm)
pick_x, pick_y, pick_z = 0.0, 0.0, -300.0
place_x, place_y, place_z = 0.0, 0.0, -300.0

is_typing = False
input_text = ""
error_msg = ""

def map_mouse_to_robot(mx, my):
    dx = mx - CENTER[0]
    dy = my - CENTER[1]
    rx = (dx / DRAW_RADIUS) * R_MAX
    ry = -(dy / DRAW_RADIUS) * R_MAX
    
    distance = math.sqrt(rx**2 + ry**2)
    if distance > R_MAX:
        rx = (rx / distance) * R_MAX
        ry = (ry / distance) * R_MAX
    return rx, ry

running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
            
        elif event.type == pygame.KEYDOWN:
            if mode == 3 and is_typing:
                if event.key == pygame.K_RETURN or event.key == pygame.K_KP_ENTER:
                    try:
                        parts = input_text.strip().split()
                        
                        # --- XỬ LÝ NHẬP LIỆU MODE 3 (Cần 6 tọa độ) ---
                        if len(parts) >= 6:
                            px, py, pz = float(parts[0]), float(parts[1]), float(parts[2])
                            lx, ly, lz = float(parts[3]), float(parts[4]), float(parts[5])
                            
                            dist_pick = math.sqrt(px**2 + py**2)
                            dist_place = math.sqrt(lx**2 + ly**2)
                            
                            if dist_pick <= R_MAX and dist_place <= R_MAX and Z_MIN <= pz <= Z_MAX and Z_MIN <= lz <= Z_MAX:
                                pick_x, pick_y, pick_z = px, py, pz
                                place_x, place_y, place_z = lx, ly, lz
                                error_msg = "Đã cập nhật tọa độ GẮP & THẢ thành công!"
                                is_typing = False
                                input_text = ""
                            else:
                                error_msg = f"Lỗi: Điểm gắp hoặc thả vượt ngoài không gian làm việc!"
                        else:
                            error_msg = "Lỗi: Cần nhập đủ 6 số (X1 Y1 Z1 X2 Y2 Z2)"
                                
                    except ValueError:
                        error_msg = "Lỗi: Chỉ được nhập số thực!"
                    
                elif event.key == pygame.K_BACKSPACE:
                    input_text = input_text[:-1]
                elif event.key == pygame.K_ESCAPE:
                    is_typing, input_text, error_msg = False, "", "Đã hủy nhập liệu."
                else:
                    if event.unicode in "0123456789- .":
                        input_text += event.unicode
                        error_msg = "" 
            else:
                # Chuyển đổi Mode
                if event.key in [pygame.K_0, pygame.K_KP0]: mode, is_typing = 0, False
                elif event.key in [pygame.K_1, pygame.K_KP1]: mode, is_typing = 1, False
                elif event.key in [pygame.K_2, pygame.K_KP2]: mode, is_typing = 2, False
                elif event.key in [pygame.K_3, pygame.K_KP3]: mode, is_typing = 3, False
                elif event.key in [pygame.K_RETURN, pygame.K_KP_ENTER] and mode == 3:
                    is_typing, input_text, error_msg = True, "", ""

        elif event.type == pygame.MOUSEWHEEL:
            if mode in [2, 3]:
                if mode == 2:
                    robot_z += event.y * z_step
                    if robot_z < Z_MIN: robot_z = Z_MIN
                    if robot_z > Z_MAX: robot_z = Z_MAX

    # Lấy tọa độ chuột cho Mode 2
    if mode == 2:
        mx, my = pygame.mouse.get_pos()
        robot_x, robot_y = map_mouse_to_robot(mx, my)

    # ================= GỬI DỮ LIỆU XUỐNG ESP32 =================
    if mode in [0, 1]:
        # CẤU TRÚC 1: Chỉ gửi mode
        msg_send = f"{mode}"
    elif mode == 2:
        # CẤU TRÚC 1: Gửi 1 điểm
        msg_send = f"2,{robot_x:.5f},{robot_y:.5f},{robot_z:.5f}"
    elif mode == 3:
        # CẤU TRÚC 2: Gửi trọn gói 2 điểm (Pick và Place)
        msg_send = f"3,{pick_x:.5f},{pick_y:.5f},{pick_z:.5f},{place_x:.5f},{place_y:.5f},{place_z:.5f}"
        
    try:
        sock.sendto(msg_send.encode('utf-8'), (UDP_IP, UDP_PORT))
    except BlockingIOError:
        pass 

    # ================= VẼ GIAO DIỆN LÊN MÀN HÌNH =================
    screen.fill((30, 30, 30)) 
    
    pygame.draw.circle(screen, (100, 100, 100), CENTER, DRAW_RADIUS, 2)
    pygame.draw.line(screen, (80, 80, 80), (CENTER[0] - DRAW_RADIUS, CENTER[1]), (CENTER[0] + DRAW_RADIUS, CENTER[1]), 1)
    pygame.draw.line(screen, (80, 80, 80), (CENTER[0], CENTER[1] - DRAW_RADIUS), (CENTER[0], CENTER[1] + DRAW_RADIUS), 1)
    
    if mode == 2:
        # Vẽ 1 điểm (Mode Manual)
        draw_x = int(CENTER[0] + (robot_x / R_MAX) * DRAW_RADIUS)
        draw_y = int(CENTER[1] - (robot_y / R_MAX) * DRAW_RADIUS)
        pygame.draw.circle(screen, (0, 255, 0), (draw_x, draw_y), 8)
    elif mode == 3:
        # Vẽ 2 điểm (Mode Pick & Place)
        dx_pick = int(CENTER[0] + (pick_x / R_MAX) * DRAW_RADIUS)
        dy_pick = int(CENTER[1] - (pick_y / R_MAX) * DRAW_RADIUS)
        pygame.draw.circle(screen, (255, 165, 0), (dx_pick, dy_pick), 8)
        
        dx_place = int(CENTER[0] + (place_x / R_MAX) * DRAW_RADIUS)
        dy_place = int(CENTER[1] - (place_y / R_MAX) * DRAW_RADIUS)
        pygame.draw.circle(screen, (0, 255, 255), (dx_place, dy_place), 8)
        
        # Vẽ đường nối nét đứt thể hiện quỹ đạo
        pygame.draw.line(screen, (150, 150, 150), (dx_pick, dy_pick), (dx_place, dy_place), 1)

    # === HIỂN THỊ THÔNG SỐ TEXT ===
    if mode == 3:
        mode_text, info_color = "PICK & PLACE (Chu trình)", (255, 165, 0)
        target_display = f"GẮP: ({pick_x:.0f}, {pick_y:.0f}, {pick_z:.0f})  --->  THẢ: ({place_x:.0f}, {place_y:.0f}, {place_z:.0f})"
    elif mode == 2:
        mode_text, info_color = "MANUAL (Theo chuột)", (0, 255, 0)
        target_display = f"Mục tiêu X: {robot_x:.1f}  |  Y: {robot_y:.1f}  |  Z: {robot_z:.1f}"
    elif mode == 1:
        mode_text, info_color, target_display = "AUTO MODE", (255, 100, 100), "Hệ thống tự sinh quỹ đạo" 
    else:
        mode_text, info_color, target_display = "BACK HOME", (255, 100, 100), "Đang giữ vị trí Home" 

    screen.blit(font.render(f"MODE: {mode_text} (Bấm 0, 1, 2, 3 để đổi)", True, info_color), (20, 20))
    screen.blit(font.render(f"[UDP OUT] {target_display}", True, (0, 255, 255)), (20, 50))
    
    # === KHUNG NHẬP LIỆU BÊN DƯỚI ===
    if mode == 3:
        if is_typing:
            pygame.draw.rect(screen, (50, 50, 50), (20, HEIGHT - 100, WIDTH - 40, 40))
            pygame.draw.rect(screen, (0, 255, 0), (20, HEIGHT - 100, WIDTH - 40, 40), 2)
            
            cursor = "|" if pygame.time.get_ticks() % 1000 < 500 else ""
            txt_input = font_large.render(f"Nhập: {input_text}{cursor}", True, (255, 255, 0))
            screen.blit(txt_input, (30, HEIGHT - 95))
            screen.blit(font.render("Nhấn ENTER để gửi lệnh xuống Robot, ESC để hủy.", True, (200, 200, 200)), (20, HEIGHT - 50))
        else:
            screen.blit(font.render("Nhấn ENTER để nhập tọa độ GẮP và THẢ (Gồm 6 số: X1 Y1 Z1 X2 Y2 Z2)", True, (200, 200, 200)), (20, HEIGHT - 50))
            
        if error_msg:
            color_err = (255, 0, 0) if "Lỗi" in error_msg else (0, 255, 0)
            screen.blit(font.render(error_msg, True, color_err), (20, HEIGHT - 130))

    elif mode == 2:
        screen.blit(font.render("Di chuột để đổi X, Y. Lăn chuột giữa để đổi Z.", True, (200, 200, 200)), (20, HEIGHT - 50))

    pygame.display.flip()
    clock.tick(50)

pygame.quit()