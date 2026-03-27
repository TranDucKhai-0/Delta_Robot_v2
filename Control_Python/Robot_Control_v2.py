import pygame
import socket
import math

# ================= CẤU HÌNH MẠNG =================
UDP_IP = "192.168.4.1" # địa chỉ của ESP32
UDP_PORT = 1234 # Port của ESP32
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setblocking(False) 

# ================= CẤU HÌNH VÙNG HOẠT ĐỘNG =================
R_MAX = 135.0      
Z_MIN = -335.0     
Z_MAX = -268.0     

# ================= CẤU HÌNH GIAO DIỆN =================
WIDTH, HEIGHT = 700, 700  
CENTER = (WIDTH // 2, HEIGHT // 2)
DRAW_RADIUS = int((min(WIDTH, HEIGHT) // 2) * 0.8)

pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Bảng Điều Khiển Robot Delta!")
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
robot_x, robot_y, robot_z = 0.0, 0.0, -300.0   

# Biến cho việc gõ phím nhập tọa độ (Dùng cho Mode 3)
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
                # Đang trong chế độ gõ chữ của Mode 3
                if event.key == pygame.K_RETURN or event.key == pygame.K_KP_ENTER:
                    # Phân tích chuỗi X Y Z khi nhấn Enter
                    try:
                        parts = input_text.strip().split()
                        if len(parts) >= 2:
                            new_x = float(parts[0])
                            new_y = float(parts[1])
                            new_z = float(parts[2]) if len(parts) == 3 else robot_z
                            
                            # Kiểm tra giới hạn an toàn
                            distance = math.sqrt(new_x**2 + new_y**2)
                            if distance <= R_MAX and Z_MIN <= new_z <= Z_MAX:
                                robot_x, robot_y, robot_z = new_x, new_y, new_z
                                error_msg = "Đã gửi tọa độ thành công!"
                                is_typing = False
                                input_text = "" # Xóa trắng sau khi nhập xong
                            else:
                                error_msg = f"Lỗi: Vượt giới hạn! (R<={R_MAX}, {Z_MIN}<=Z<={Z_MAX})"
                        else:
                            error_msg = "Lỗi: Hãy nhập ít nhất X và Y."
                    except ValueError:
                        error_msg = "Lỗi: Chỉ được nhập số!"
                    
                elif event.key == pygame.K_BACKSPACE:
                    input_text = input_text[:-1]
                    error_msg = ""
                elif event.key == pygame.K_ESCAPE:
                    is_typing = False
                    input_text = ""
                    error_msg = "Đã hủy nhập liệu."
                else:
                    # Chỉ cho phép nhập số, dấu trừ, dấu chấm và khoảng trắng
                    if event.unicode in "0123456789- .":
                        input_text += event.unicode
                        error_msg = "" # Xóa thông báo lỗi khi bắt đầu gõ lại
            else:
                # Chế độ bình thường (chọn Mode)
                if event.key == pygame.K_0 or event.key == pygame.K_KP0:
                    mode = 0
                    is_typing = False
                elif event.key == pygame.K_1 or event.key == pygame.K_KP1:
                    mode = 1
                    is_typing = False
                elif event.key == pygame.K_2 or event.key == pygame.K_KP2:
                    mode = 2
                    is_typing = False
                elif event.key == pygame.K_3 or event.key == pygame.K_KP3:
                    mode = 3
                    is_typing = False
                elif event.key == pygame.K_RETURN or event.key == pygame.K_KP_ENTER:
                    if mode == 3:
                        is_typing = True
                        input_text = ""
                        error_msg = ""
                        
        elif event.type == pygame.MOUSEWHEEL:
            # Vẫn giữ tính năng lăn chuột để chỉnh Z cho tiện ở cả Mode 2 và 3
            if mode in [2, 3]:
                robot_z += event.y * z_step
                if robot_z < Z_MIN: robot_z = Z_MIN
                if robot_z > Z_MAX: robot_z = Z_MAX

    # Lấy tọa độ chuột nếu đang ở Mode 2
    if mode == 2:
        mx, my = pygame.mouse.get_pos()
        robot_x, robot_y = map_mouse_to_robot(mx, my)

    # ================= GỬI DỮ LIỆU XUỐNG ESP32 =================
    if mode in [0, 1]:
        msg_send = f"{mode}"
    else:
        msg_send = f"{mode},{robot_x:.7f},{robot_y:.7f},{robot_z:.7f}"
        
    try:
        sock.sendto(msg_send.encode('utf-8'), (UDP_IP, UDP_PORT))
    except BlockingIOError:
        pass 

    # ================= VẼ GIAO DIỆN LÊN MÀN HÌNH =================
    screen.fill((30, 30, 30)) 
    
    # Vẽ vòng tròn giới hạn vùng làm việc
    pygame.draw.circle(screen, (100, 100, 100), CENTER, DRAW_RADIUS, 2)
    pygame.draw.line(screen, (80, 80, 80), (CENTER[0] - DRAW_RADIUS, CENTER[1]), (CENTER[0] + DRAW_RADIUS, CENTER[1]), 1)
    pygame.draw.line(screen, (80, 80, 80), (CENTER[0], CENTER[1] - DRAW_RADIUS), (CENTER[0], CENTER[1] + DRAW_RADIUS), 1)
    
    # Vẽ điểm mục tiêu hiện tại
    target_color = (0, 255, 255) if mode in [2, 3] else (150, 150, 150)
    draw_target_x = int(CENTER[0] + (robot_x / R_MAX) * DRAW_RADIUS)
    draw_target_y = int(CENTER[1] - (robot_y / R_MAX) * DRAW_RADIUS)
    
    pygame.draw.circle(screen, target_color, (draw_target_x, draw_target_y), 8)
    pygame.draw.circle(screen, (255, 255, 255), (draw_target_x, draw_target_y), 15, 1)

    # === HIỂN THỊ THÔNG SỐ TEXT ===
    if mode == 3:
        mode_text = "KEYBOARD (Nhập Phím) - "
        info_color = (255, 165, 0)
        target_display = f"X: {robot_x:.2f}  |  Y: {robot_y:.2f}  |  Z: {robot_z:.2f}"
    elif mode == 2:
        mode_text = "MANUAL (Theo chuột) - "
        info_color = (0, 255, 0)
        target_display = f"X: {robot_x:.2f}  |  Y: {robot_y:.2f}  |  Z: {robot_z:.2f}"
    elif mode == 1:
        mode_text = f"AUTO MODE - "
        info_color = (255, 100, 100)
        target_display = "Ngừng gửi tọa độ x,y,z" 
    else:
        mode_text = f"BACK HOME - "
        info_color = (255, 100, 100)
        target_display = "Ngừng gửi tọa độ x,y,z" 

    txt_mode = font.render(f"MODE: {mode_text} (Bấm 0, 1, 2, 3 để đổi)", True, info_color)
    txt_target = font.render(f"[MỤC TIÊU ĐANG GỬI] {target_display}", True, (0, 255, 255))
    
    screen.blit(txt_mode, (20, 20))
    screen.blit(txt_target, (20, 50))
    
    # === KHUNG NHẬP LIỆU BÊN DƯỚI (Chỉ xuất hiện ở Mode 3) ===
    if mode == 3:
        if is_typing:
            # Vẽ hộp thoại nổi bật
            pygame.draw.rect(screen, (50, 50, 50), (20, HEIGHT - 100, WIDTH - 40, 40))
            pygame.draw.rect(screen, (0, 255, 0), (20, HEIGHT - 100, WIDTH - 40, 40), 2)
            
            # Con trỏ nhấp nháy
            cursor = "|" if pygame.time.get_ticks() % 1000 < 500 else ""
            txt_input = font_large.render(f"Nhập X Y Z: {input_text}{cursor}", True, (255, 255, 0))
            screen.blit(txt_input, (30, HEIGHT - 95))
            
            txt_guide = font.render("Nhấn ENTER để xác nhận, ESC để hủy.", True, (200, 200, 200))
            screen.blit(txt_guide, (20, HEIGHT - 50))
        else:
            txt_guide = font.render("Nhấn ENTER để bắt đầu nhập tọa độ X Y Z. (Hoặc lăn chuột đổi Z)", True, (200, 200, 200))
            screen.blit(txt_guide, (20, HEIGHT - 50))
            
        if error_msg:
            color_err = (255, 0, 0) if "Lỗi" in error_msg else (0, 255, 0)
            txt_err = font.render(error_msg, True, color_err)
            screen.blit(txt_err, (20, HEIGHT - 130))
            
    elif mode == 2:
        # Nhắc nhở lăn chuột cho Mode 2
        txt_guide = font.render("Di chuột để đổi X, Y. Lăn chuột giữa để đổi Z.", True, (200, 200, 200))
        screen.blit(txt_guide, (20, HEIGHT - 50))

    pygame.display.flip()
    clock.tick(50)

pygame.quit()