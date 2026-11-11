# หลักฐานการตรวจ Mini-Project 2

## ชุดหลักหลัง merge

`post-merge-5fbcb76/` เป็นผลจากการตรวจ source บน branch `main` หลัง PR #11 ถูก merge ด้วย commit `5fbcb7679298f36e2bae8b7cd3dbe6bf6a609800` วันที่ 4 พฤศจิกายน 2569

- Environment: Lima, Ubuntu 24.04.5, Linux 6.8.0-142-generic, ARM64, `CONFIG_KPROBES=y`, `CONFIG_KRETPROBES=y`, `CONFIG_BSD_PROCESS_ACCT=y`
- Build/load: `make clean && make all && make load` สำเร็จ
- Demo: จบด้วย exit 0; ภาพ `03-demo-terminal.png` และ transcript แสดง run PID 2508, 16,521 minor และ 8,193 major faults ซึ่งตรงกับ `getrusage()` หลังหัก baseline
- Regression follow-up: รันบน Linux 6.8.0-147-generic; ทั้ง 5 checks ผ่าน รวม PID reuse (PID 26873) ภาพกับ transcript มาจากรอบเดียวกัน ส่วนหลักฐานชุดเดิมบันทึก Linux 6.8.0-142-generic ไว้ใน `00-environment.txt`
- Cleanup: `SIGCHLD=SIG_IGN` ที่สืบทอดจาก parent ผ่าน; ภาพและ transcript ของ PID 2513 แสดง 125 minor faults, target กลับ idle และ follow กลับเป็น N
- Experiments: E1–E8 จบด้วย exit status 0; ผลเทียบ process-only กับ waited descendants อธิบายตาม scope
- Make clean: ลบ test binaries หลังจบการตรวจ

## ตรวจซ้ำ PR head ล่าสุด

`recheck-2026-11-11/` เป็นผลตรวจจาก PR head `ed0a7a64bb9a9a22aa93e97d2012740f465cae5a` หลังปรับรายงานและโฟลเดอร์หน้าปก โดยใช้ Lima, Ubuntu 24.04.5 LTS, Linux 6.8.0-147-generic, ARM64 และ GCC 13.3.0

- `make clean`, `make all`, `make load`: สำเร็จและสร้าง proc entries ครบ
- Demo: exit 0; 16,518 minor และ 8,193 major faults ตรงกับ `getrusage()` หลังหัก baseline
- Regression: ผ่านทั้ง 5 checks; PID 22425 ถูก reuse และตรงกับภาพ Terminal ใน `terminal/02-regression-terminal.png`
- Experiments: E1–E8 จบด้วย exit status 0
- Live process: VMA 30 รายการ, virtual size และ page-fault counts ตรงกับ procfs
- E7 timeline แยก: exit 0; 16,522 minor และ 8,193 major faults ตรงกับ `getrusage()`
- SIGCHLD cleanup: child exit 0; หลังจบ `follow=N`, `target=0`, `pid: 0`, `state: idle`
- LaTeX: build สำเร็จเป็น PDF A4 จำนวน 21 หน้า; `terminal/01-latex-build-terminal.png` แสดง metadata และ exit status จากรอบเดียวกับ `09-latex-build.txt`
- Final cleanup: unload โมดูล, `make clean`, และหยุด Lima สำเร็จ; บันทึกอยู่ใน `10-cleanup.txt`

Kbuild แสดง warning ว่าชื่อ executable ของ compiler ต่างกัน (`aarch64-linux-gnu-gcc-13` กับ `gcc-13`) แต่ทั้งคู่เป็น GCC 13.3.0. BTF generation ถูกข้ามเพราะ VM ไม่มี `vmlinux`; โมดูลยัง build และ load สำเร็จ ผลชุดนี้เป็นการยืนยันการทำงานของ PR head ปัจจุบัน ส่วนค่าที่ใช้ทำตารางและกราฟ E1–E8 ในรายงานยังเป็น measurement baseline จาก Linux 6.8.0-142-generic

`00-environment.txt` ถึง `10-cleanup.txt` เก็บ log ของขั้นตอนหลัก และ `experiment-results/` เก็บ output แยกของ E1–E8. ภาพ Terminal สองภาพใน `terminal/` เป็นภาพ crop จากหน้าต่าง Terminal โดยตรง และแต่ละภาพตรงกับผลใน log ของรอบเดียวกัน. ระหว่างปิด Lima มี host agent บันทึกข้อความ `use of closed network connection` ขณะปิด connection แล้วรายงานต่อว่า instance หยุดสำเร็จ; คำสั่งจบด้วย exit status 0. ชุดนี้ยืนยันการรันซ้ำ ไม่ได้แทน measurement baseline ในรายงาน.

ไฟล์ระดับบนเก็บ log ของ environment, build/load, demo, regression, cleanup และ experiments ส่วน `experiment-results/` เก็บผลดิบของแต่ละกรณี E1–E8

## หลักฐานที่รวมในรายงาน

ภาพทั้งแปดใบเป็นภาพจับหน้าจอที่ครอปเฉพาะผลลัพธ์ใน Terminal ไม่มีภาพเต็มเดสก์ท็อป และแสดงหลักฐานของแต่ละช่วง ภาพ demo, regression, E7 และ SIGCHLD cleanup จับจากรอบเดียวกับ transcript ของแต่ละงาน โดย PID และผลลัพธ์ตรงกัน ไฟล์ `.txt` เก็บผลที่บันทึกไว้ รวมคำสั่ง PID เวลา exit status และค่าผลลัพธ์เท่าที่มีในแต่ละรายการ ส่วนรายงาน `pfstat` ของ shell PID 1622 มีเฉพาะภาพ

| ไฟล์ | หลักฐานที่แสดง | ตำแหน่งในรายงาน |
|---|---|---|
| `terminal/01-kernel-options-terminal.png` | kernel options ที่จำเป็นสำหรับ kprobes และ process accounting; kernel version กับ architecture อยู่ใน environment log | Problem 2, Step 1: Test environment and method |
| `terminal/02-build-load-terminal.png` | การโหลดโมดูลและ proc interfaces | Problem 2, Step 1: Build and load |
| `terminal/03-demo-terminal.png` | interval samples และ exit summary จาก demo run PID 2508 (16,521 minor / 8,193 major), ตรงกับ `02-demo-terminal-capture.txt` | Problem 2, Step 1: End-to-end demo |
| `terminal/04-regression-terminal.png` | ผล regression ทั้ง 5 รายการบน Linux 6.8.0-147-generic; PID 26873 ถูก reuse และตรงกับ `03-regression.txt` | Problem 2, Step 3: Post-merge checks |
| `terminal/05-pfstat-report-terminal.png` | VMA, virtual size, RSS, fault counts และ mapping groups ของ bash PID 1622 (มีเฉพาะภาพ ไม่มี transcript) | Problem 1, Step 2: How the tool works |
| `terminal/06-experiments-terminal.png` | รายการกรณี E1–E8 ที่สคริปต์ทดลองเรียกใช้ | Problem 2, Step 1: How we checked the results |
| `terminal/07-e7-timeline-terminal.png` | interval samples และ exit summary จาก E7 run PID 2286, ตรงกับ `experiment-results/e7-timeline.txt` | Problem 2, Step 2: E7 virtual size and RSS over time |
| `terminal/08-sigchld-cleanup-terminal.png` | `pfcount run` ของ PID 2513 และสถานะหลัง cleanup: `follow=N`, `pid: 0`, `state: idle` | Problem 2, Step 3: Post-merge checks |

`02-demo-terminal-capture.txt` เป็น transcript ฉบับเต็มของ demo run PID 2508 (16,521 minor / 8,193 major faults) พร้อม workload checkpoints, interval samples และ rates ภาพและ transcript เป็นการรันรอบเดียวกัน ส่วน `03-regression.txt` และภาพ regression แสดงผลจากรอบเดียวกันบน Linux 6.8.0-147-generic (PID 26873) `experiment-results/e7-timeline.txt` เป็น transcript ของ E7 run PID 2286 ซึ่งรันคำสั่งเดียวกันแยกจากรอบสคริปต์ที่ได้ E1–E6 และ E8 (PID 10605–10663) และ `04-sigchld-cleanup.txt` เก็บผล run PID 2513 ที่รับ `SIGCHLD=SIG_IGN` มาจาก parent; ทั้งสองรายการมีภาพ Terminal ที่ตรงกับตัวเลขและสถานะใน transcript

log review ก่อน merge ไม่รวมอยู่ในชุดส่งงาน

## ข้อจำกัด

- ไม่มี GitHub CI checks ตั้งไว้สำหรับ branch นี้ ผลเป็นการ build/test ด้วยตนเองใน Lima VM
- ทดสอบเฉพาะ Linux 6.8 ARM64 และ configuration ที่บันทึกไว้
- ไม่ได้ทดสอบใน UTM; โจทย์ไม่ได้กำหนดให้ใช้ UTM
