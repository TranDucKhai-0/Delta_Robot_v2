% --- CÁC THAM SỐ ĐẦU VÀO ---
R = 137;         % Bán kính (mm)
Z_MAX = -280;    % Tọa độ Z tối đa
Z_MIN = -350;    % Tọa độ Z tối thiểu

% --- TẠO DỮ LIỆU HÌNH TRỤ ---
[X, Y, Z] = cylinder(R, 100);
Z = Z * (Z_MAX - Z_MIN) + Z_MIN;

% --- VẼ ĐỒ THỊ ---
figure;
hold on; 

% 1. Vẽ thân và 2 nắp hình trụ (màu xanh dương)
surf(X, Y, Z, 'FaceColor', 'blue', 'EdgeColor', 'none');
fill3(X(1,:), Y(1,:), Z(1,:), 'blue', 'EdgeColor', 'none');
fill3(X(2,:), Y(2,:), Z(2,:), 'blue', 'EdgeColor', 'none');

% 2. Mở rộng không gian và vẽ mặt phẳng xOy (Z = 0)
% Ép giới hạn trục Z chạy từ dưới đáy hình trụ lên vượt qua 0 một chút
zlim([Z_MIN - 100, -40]);

% --- THIẾT LẬP ÁNH SÁNG VÀ HIỂN THỊ ---
axis equal;
grid on;
view(3);

camlight('headlight'); 
camlight('left');
lighting gouraud; 

xlabel('Trục X (mm)');
ylabel('Trục Y (mm)');
zlabel('Trục Z (mm)');
title('Khối trụ đặc và mặt phẳng xOy tại Z = 0');

hold off;