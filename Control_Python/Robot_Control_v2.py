import pygame
import socket
import math

# ==============================================================================
# 1. CẤU HÌNH MẠNG & HỆ THỐNG
# ==============================================================================
UDP_IP = "192.168.4.1"
UDP_PORT = 1234
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setblocking(False)

# ==============================================================================
# 2. CẤU HÌNH ROBOT & GIAO DIỆN
# ==============================================================================
R_MAX = 137.0
Z_MIN = -350.0
Z_MAX = -280.0

WIDTH, HEIGHT = 700, 700
CENTER = (WIDTH // 2, HEIGHT // 2)
DRAW_RADIUS = int((min(WIDTH, HEIGHT) // 2) * 0.8)

# ==============================================================================
# 3. KHỞI TẠO PYGAME
# ==============================================================================
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

# ==============================================================================
# 4. BIẾN TRẠNG THÁI (STATE) & QUẢN LÝ GÓI TIN UDP
# ==============================================================================
mode = 0
z_step = 5.0

# Tọa độ cho Mode 2 (Manual)
robot_x, robot_y, robot_z = 0.0, 0.0, -300.0

# Tọa độ cho Mode 3 (Pick & Place)
pick_x, pick_y, pick_z = 0.0, 0.0, -300.0
place_x, place_y, place_z = 0.0, 0.0, -300.0

# Trạng thái nhập liệu Mode 3
is_typing = False
input_text = ""
error_msg = ""

# --- LOGIC QUẢN LÝ ID VÀ SỐ LẦN GỬI ---
msg_id = 0
send_count = 0
MAX_SEND_TIMES = 10

def trigger_new_command():
    """Lật ID (0 <-> 1) và đặt lại biến đếm để phát 10 gói tin mới."""
    global msg_id, send_count
    msg_id = 1 - msg_id  # Lật giá trị 0 thành 1, 1 thành 0
    send_count = MAX_SEND_TIMES

# ==============================================================================
# 5. CÁC HÀM HỖ TRỢ (HELPER FUNCTIONS)
# ==============================================================================
def map_mouse_to_robot(mx, my):
    """Chuyển đổi tọa độ chuột trên màn hình thành tọa độ không gian làm việc của Robot."""
    dx = mx - CENTER[0]
    dy = my - CENTER[1]
    
    rx = (dx / DRAW_RADIUS) * R_MAX
    ry = -(dy / DRAW_RADIUS) * R_MAX
    
    distance = math.sqrt(rx**2 + ry**2)
    if distance > R_MAX:
        rx = (rx / distance) * R_MAX
        ry = (ry / distance) * R_MAX
        
    return rx, ry

def parse_mode3_input(text):
    """Xử lý và kiểm tra tính hợp lệ của chuỗi tọa độ nhập vào ở Mode 3."""
    parts = text.strip().split()
    if len(parts) < 6:
        return False, "Lỗi: Cần nhập đủ 6 số (X1 Y1 Z1 X2 Y2 Z2)"
        
    try:
        px, py, pz = float(parts[0]), float(parts[1]), float(parts[2])
        lx, ly, lz = float(parts[3]), float(parts[4]), float(parts[5])
        
        dist_pick = math.sqrt(px**2 + py**2)
        dist_place = math.sqrt(lx**2 + ly**2)
        
        if dist_pick > R_MAX or dist_place > R_MAX:
            return False, "Lỗi: Điểm gắp/thả vượt ngoài bán kính R_MAX!"
        if not (Z_MIN <= pz <= Z_MAX) or not (Z_MIN <= lz <= Z_MAX):
            return False, f"Lỗi: Trục Z phải nằm trong khoảng [{Z_MIN}, {Z_MAX}]!"
            
        return True, (px, py, pz, lx, ly, lz)
    except ValueError:
        return False, "Lỗi: Chỉ được nhập số thực!"

# Lúc khởi động chương trình, gửi tín hiệu Mode 0 đi 1 lần
trigger_new_command()

# ==============================================================================
# 6. VÒNG LẶP CHÍNH (MAIN LOOP)
# ==============================================================================
running = True
while running:
    # --------------------------------------------------------------------------
    # A. XỬ LÝ SỰ KIỆN TỪ BÀN PHÍM VÀ CHUỘT
    # --------------------------------------------------------------------------
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
            
        elif event.type == pygame.KEYDOWN:
            # -- Đang gõ văn bản trong Mode 3 --
            if mode == 3 and is_typing:
                if event.key in (pygame.K_RETURN, pygame.K_KP_ENTER):
                    success, result = parse_mode3_input(input_text)
                    if success:
                        pick_x, pick_y, pick_z, place_x, place_y, place_z = result
                        error_msg = "Đã cập nhật tọa độ GẮP & THẢ thành công!"
                        is_typing = False
                        input_text = ""
                        trigger_new_command() # Phát lệnh Pick & Place
                    else:
                        error_msg = result
                        
                elif event.key == pygame.K_BACKSPACE:
                    input_text = input_text[:-1]
                elif event.key == pygame.K_ESCAPE:
                    is_typing, input_text, error_msg = False, "", "Đã hủy nhập liệu."
                else:
                    if event.unicode in "0123456789- .":
                        input_text += event.unicode
                        error_msg = "" 
            
            # -- Phím tắt chuyển đổi Mode --
            else:
                if event.key in (pygame.K_0, pygame.K_KP0):
                    mode, is_typing = 0, False
                    trigger_new_command()
                elif event.key in (pygame.K_1, pygame.K_KP1):
                    mode, is_typing = 1, False
                    trigger_new_command()
                elif event.key in (pygame.K_2, pygame.K_KP2):
                    mode, is_typing = 2, False
                    trigger_new_command()
                elif event.key in (pygame.K_3, pygame.K_KP3):
                    mode, is_typing = 3, False
                    
                    # Tự động gán tọa độ Home làm Default khi vào Mode 3
                    home_z = Z_MAX - 1.0
                    pick_x, pick_y, pick_z = 0.0, 0.0, home_z
                    place_x, place_y, place_z = 0.0, 0.0, home_z
                    error_msg = "Đã tải tọa độ Home mặc định."
                    trigger_new_command() # Phát lệnh ngay khi chuyển Mode
                
                elif event.key in (pygame.K_RETURN, pygame.K_KP_ENTER) and mode == 3:
                    is_typing, input_text, error_msg = True, "", ""

        # -- Xử lý cuộn chuột cho Mode 2 --
        elif event.type == pygame.MOUSEWHEEL:
            if mode == 2:
                robot_z += event.y * z_step
                robot_z = max(Z_MIN, min(robot_z, Z_MAX))
                trigger_new_command() # Cập nhật Z mới

    # Liên tục kiểm tra tọa độ chuột cho Mode 2
    if mode == 2:
        mx, my = pygame.mouse.get_pos()
        new_x, new_y = map_mouse_to_robot(mx, my)
        # Chỉ cập nhật và phát UDP nếu tọa độ chuột thực sự thay đổi
        if new_x != robot_x or new_y != robot_y:
            robot_x, robot_y = new_x, new_y
            trigger_new_command()

    # --------------------------------------------------------------------------
    # B. XỬ LÝ ĐÓNG GÓI & GỬI DỮ LIỆU UDP (CHỈ GỬI KHI send_count > 0)
    # --------------------------------------------------------------------------
    if send_count > 0:
        msg_send = ""
        # NHÉT msg_id VÀO VỊ TRÍ THỨ 2 TRONG CHUỖI CHO TẤT CẢ CÁC MODE
        if mode in [0, 1]:
            msg_send = f"{mode},{msg_id}"
        elif mode == 2:
            msg_send = f"2,{msg_id},{robot_x:.5f},{robot_y:.5f},{robot_z:.5f}"
        elif mode == 3:
            msg_send = f"3,{msg_id},{pick_x:.5f},{pick_y:.5f},{pick_z:.5f},{place_x:.5f},{place_y:.5f},{place_z:.5f}"

        if msg_send:
            try:
                sock.sendto(msg_send.encode('utf-8'), (UDP_IP, UDP_PORT))
                send_count -= 1
            except BlockingIOError:
                pass 

    # --------------------------------------------------------------------------
    # C. CẬP NHẬT GIAO DIỆN MÀN HÌNH (UI)
    # --------------------------------------------------------------------------
    screen.fill((30, 30, 30))
    
    pygame.draw.circle(screen, (100, 100, 100), CENTER, DRAW_RADIUS, 2)
    pygame.draw.line(screen, (80, 80, 80), (CENTER[0] - DRAW_RADIUS, CENTER[1]), (CENTER[0] + DRAW_RADIUS, CENTER[1]), 1)
    pygame.draw.line(screen, (80, 80, 80), (CENTER[0], CENTER[1] - DRAW_RADIUS), (CENTER[0], CENTER[1] + DRAW_RADIUS), 1)
    
    if mode == 2:
        draw_x = int(CENTER[0] + (robot_x / R_MAX) * DRAW_RADIUS)
        draw_y = int(CENTER[1] - (robot_y / R_MAX) * DRAW_RADIUS)
        pygame.draw.circle(screen, (0, 255, 0), (draw_x, draw_y), 8)
        
    elif mode == 3:
        dx_pick = int(CENTER[0] + (pick_x / R_MAX) * DRAW_RADIUS)
        dy_pick = int(CENTER[1] - (pick_y / R_MAX) * DRAW_RADIUS)
        pygame.draw.circle(screen, (255, 165, 0), (dx_pick, dy_pick), 8)
        
        dx_place = int(CENTER[0] + (place_x / R_MAX) * DRAW_RADIUS)
        dy_place = int(CENTER[1] - (place_y / R_MAX) * DRAW_RADIUS)
        pygame.draw.circle(screen, (0, 255, 255), (dx_place, dy_place), 8)
        
        pygame.draw.line(screen, (150, 150, 150), (dx_pick, dy_pick), (dx_place, dy_place), 1)

    # Hiển thị text trạng thái
    if mode == 3:
        mode_text, info_color = "PICK & PLACE (Chu trình)", (255, 165, 0)
        target_display = f"GẮP: ({pick_x:.0f}, {pick_y:.0f}, {pick_z:.0f})   --->   THẢ: ({place_x:.0f}, {place_y:.0f}, {place_z:.0f})"
    elif mode == 2:
        mode_text, info_color = "MANUAL (Theo chuột)", (0, 255, 0)
        target_display = f"Mục tiêu X: {robot_x:.1f}  |  Y: {robot_y:.1f}  |  Z: {robot_z:.1f}"
    elif mode == 1:
        mode_text, info_color, target_display = "AUTO MODE", (255, 100, 100), "Hệ thống tự sinh quỹ đạo" 
    else:
        mode_text, info_color, target_display = "BACK HOME", (255, 100, 100), "Đang giữ vị trí Home" 

    screen.blit(font.render(f"MODE: {mode_text} (Bấm 0, 1, 2, 3 để đổi)", True, info_color), (20, 20))
    screen.blit(font.render(f"[UDP OUT] {target_display} | ID Gói: {msg_id}", True, (0, 255, 255)), (20, 50))
    
    # Giao diện thông báo mạng (Dùng để monitor việc spam gói tin)
    if send_count > 0:
        screen.blit(font.render(f"Đang đồng bộ lệnh mới... (Còn {send_count} gói)", True, (0, 255, 0)), (WIDTH - 350, 20))
    else:
        screen.blit(font.render("Idle (Không gửi mạng)", True, (100, 100, 100)), (WIDTH - 250, 20))

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
