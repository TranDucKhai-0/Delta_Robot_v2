clear; clc; close all;

% --- KÍCH THƯỚC BƯỚC QUÉT (ĐỘ MỊN) ---
% Tăng/Giảm số này để cắt lớp mỏng hơn hình sẽ mịn hơn
STEP_Z  = 2.0;  % Cắt lớp Z
STEP_XY = 3.0;  % Quét lưới XY

% --- (VÙNG QUÉT) ---
X_MIN = -300; X_MAX = 300;
Y_MIN = -300; Y_MAX = 300;
Z_MIN = -500; Z_MAX = -120;

% --- GIỚI HẠN GÓC (RAD) ---
RAD_MIN = -pi/2;
RAD_MAX = 0;

% --- KIỂM TRA FK ---
VERIFY_FK = true;
FK_TOL = 1e-2;

% --- CẤU HÌNH VẼ MẶT (SURFACE) ---
% Alpha càng nhỏ thì bề mặt càng ôm sát chi tiết, nhưng nếu nhỏ quá sẽ bị thủng lỗ.
ALPHA_VAL = 15; 

% =========================
% SCAN (QUÉT LƯỚI CẮT LỚP)
% =========================
fprintf('Đang khởi tạo lưới quét...\n');
fprintf('Step Z: %.1f mm | Step XY: %.1f mm\n', STEP_Z, STEP_XY);

validList = []; % Chứa các điểm hợp lệ [x, y, z]

% Tạo dãy Z để cắt lớp
z_layers = Z_MIN : STEP_Z : Z_MAX;
total_layers = length(z_layers);

% Thanh tiến trình
wb = waitbar(0, 'Đang quét vùng làm việc...');

t_start = tic;

for k = 1:total_layers
    z_curr = z_layers(k);
    
    % Tạo lưới X-Y tại lớp Z hiện tại
    [x_grid, y_grid] = meshgrid(X_MIN:STEP_XY:X_MAX, Y_MIN:STEP_XY:Y_MAX);
    
    % Chuyển thành vector cột để xử lý nhanh
    X_col = x_grid(:);
    Y_col = y_grid(:);
    Z_col = repmat(z_curr, size(X_col));
    
    n_points = length(X_col);
    layer_valid_pts = zeros(n_points, 3);
    cnt_layer = 0;
    
    for i = 1:n_points
        % 1. Inverse Kinematics
        th = IK(X_col(i), Y_col(i), Z_col(i)); 
        
        if isempty(th), continue; end

        % 2. Check Joint Limits
        if any(th < RAD_MIN) || any(th > RAD_MAX)
            continue;
        end

        % 3. Verify Forward Kinematics (Optional but recommended)
        if VERIFY_FK
            p = FK(th);
            if isempty(p), continue; end
            err = norm(p - [X_col(i), Y_col(i), Z_col(i)]);
            if ~isfinite(err) || err > FK_TOL
                continue;
            end
        end
        
        % Lưu điểm hợp lệ
        cnt_layer = cnt_layer + 1;
        layer_valid_pts(cnt_layer, :) = [X_col(i), Y_col(i), Z_col(i)];
    end
    
    % Gom kết quả của lớp này vào danh sách tổng
    if cnt_layer > 0
        validList = [validList; layer_valid_pts(1:cnt_layer, :)];
    end
    
    % Cập nhật tiến trình
    waitbar(k/total_layers, wb, sprintf('Đang quét lớp Z = %.1f (Found: %d)', z_curr, size(validList, 1)));
end
close(wb);
t_end = toc(t_start);

if isempty(validList)
    error("Không tìm thấy điểm hợp lệ nào. Kiểm tra lại Box, IK hoặc Limits.");
end

fprintf("HOÀN TẤT. Tổng điểm tìm thấy: %d\n", size(validList, 1));
fprintf("Thời gian quét: %.2f giây.\n", t_end);

% Lưu dữ liệu (chỉ lưu tọa độ XYZ để nhẹ máy)
save("workspace_xyz_grid.mat", "validList");

% =========================
% PLOT: SURFACE ONLY
% =========================
figure('Color','w'); hold on; grid on; axis equal;
xlabel("X (mm)"); ylabel("Y (mm)"); zlabel("Z (mm)");
title(sprintf("Workspace Boundary (Grid Scan) | StepZ=%.1f", STEP_Z));
view(3);

% --- VẼ HÌNH TRỤ THAM CHIẾU ---
R_cyl = 137; Zcyl_max = -280; Zcyl_min = -350;


theta = linspace(0, 2*pi, 80);
xC = R_cyl*cos(theta); yC = R_cyl*sin(theta);
plot3(xC, yC, Zcyl_min*ones(size(theta)), 'r-', 'LineWidth', 2);
plot3(xC, yC, Zcyl_max*ones(size(theta)), 'r-', 'LineWidth', 2);
for t = linspace(0, 2*pi, 12)
    plot3([R_cyl*cos(t) R_cyl*cos(t)], [R_cyl*sin(t) R_cyl*sin(t)], ...
          [Zcyl_min Zcyl_max], 'r-', 'LineWidth', 1);
end

% --- VẼ BỀ MẶT WORKSPACE (KHÔNG VẼ POINT) ---
if size(validList, 1) > 4
    % Sử dụng alphaShape để tạo khối bao quanh các điểm
    % alphaShape tốt hơn boundary() đối với các vùng lõm (concave)
    shp = alphaShape(validList(:,1), validList(:,2), validList(:,3), ALPHA_VAL);
    
    % Lấy các mặt tam giác biên
    [tri, P] = boundaryFacets(shp);
    
    % Vẽ bề mặt
    % FaceAlpha = 0.6: Độ trong suốt để nhìn thấy hình trụ bên trong
    % EdgeColor = 'none': Bỏ lưới tam giác đen cho mượt
    trisurf(tri, P(:,1), P(:,2), P(:,3), ...
        'FaceColor', [0 0.5 1], ...   % Màu xanh dương
        'FaceAlpha', 0.6, ...         % Độ trong suốt
        'EdgeColor', 'none', ...      % Không vẽ viền lưới
        'AmbientStrength', 0.5);      % Tăng độ bóng
        
    camlight; lighting gouraud; % Thêm ánh sáng cho khối 3D đẹp hơn
else
    warning('Không đủ điểm để tạo bề mặt 3D.');
end

fprintf("Đã vẽ xong bề mặt.\n");