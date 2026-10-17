Kenzie Darrel Ivander Tyaga Pangala
5025261085

Analisis Singkat & Kendala :
1. Menentukan range scalar untuk thresholding 
2. Delay kamera dibawah 30 fps dan sering drop ketika calib (unsolved)
3. Menentukan metode agar objek bisa dikenali dan diberi label

Lebar Asli Objek:
1. 21.0 --> red paper
2. 20.8 --> green paper
3. 11.5 --> blue eraser
Kamera yang saya gunakan adalah kamera HP yang dihubungkan ke laptop dengan menggunakan IP webcam

Penjelasan calib
Video yang ditangkap oleh kamera akan dikonversi menjadi frame dalam loop dan frame akan dikonversi menjadi warna abu abu menggunakan cv::cvtColor. Dari hasil konversi warna tersebut dipanggil fungsi cv::findChessboardCorners untuk mencari checkerboard dan fungsi cv::drawChessboardCorners untuk menggambar line yang menghubungkan titik-titik sudut pada checkerboard. Setelah loop selesai/kamera mati dipanggil fungsi cv::calibrateCamera untuk menghitung hasil kalibrasi dan hasilnya akan dimasukkan ke dalam yaml dengan fungsi cv::FileStorage::WRITE.

Penjelasan main
main akan mengambil fx, fy, cx, cy dari hasil kalibrasi dengan fungsi cv::FileStorage::READ. Video yang ditangkap oleh kamera akan dikonversi menjadi frame dalam loop dan frame akan dikonversi menjadi warna dalam hls menggunakan cv::cvtColor. Dari hasil konversi warna tersebut dilakukan thresholding untuk ketiga objek yaitu merah, hijau, dan biru yang dimasukkan ke dalam vector untuk membedakan setiap objek. Lalu morphology dilakukan untuk setiap threshold dalam vector dan dipanggil fungsi cv::findContours untuk mencari contour yang terbentuk setelah morpholgy serta dibentuk bounding box berdasarkan countourArea yang memenuhi kondisi. Dari setiap bounding box yang dihasilkan dideklarasikan lebar bounding box sebagai w pixel dan ditentukan centroid/titik pusat objek dengan menggunakan cv::moment. Dilakukan perhitungan untuk koordinat (x,y,z) sesuai rumus
z = fx * w_real / w_pixel
y = (y_pixel - cy) / fy
x = (x_pixel - cx) / fx
hasil perhitungan akan ditampilkan dengan menggunakan fungsi cv::putText
