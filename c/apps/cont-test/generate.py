import os
import sys
import random

if len(sys.argv) <3:
    print("Missing arguments")
    exit()

# The minimum address to use on the target machine
target_min_addr = int(sys.argv[1])
# The maximum address to use on the target machine
target_max_addr = int(sys.argv[2])

# How many accesses to include in each cores program
window_size = 10000
if len(sys.argv) >=4:
    window_size = int(sys.argv[3])

# Randomization seed
if len(sys.argv) >=5:
    random.seed(int(sys.argv[4]))


if (target_min_addr % 4) != 0 or (target_max_addr % 4) != 0:
    print("Invalid target address range")
    exit()
    
target_idx_range = int((target_max_addr - target_min_addr) / 4)
    
trace_files = [
    "accesses_0.txt",
    "accesses_1.txt",
    "accesses_2.txt",
    "accesses_3.txt",
    ]
    
trace_max_addr=0
for file in trace_files:
    with open(file, 'r') as accesses_file:
        for line in accesses_file:
            split = line.strip().split(',')
            addr = int(split[0])
            
            if (addr % 4) != 0:
                print(f"Unaligned address in '{file}': {addr}")
                exit()
                
            if addr > trace_max_addr:
                trace_max_addr = addr

# Tried to do index range "squishing" where the trace memory space is squished into the memory space of the target
# such that addresses are still positioned the same place relative to each other.
# However, because the target space is 0,19 % of the original range, all sequential accesses of close addresses get
# squished to the same addres, meaning there is very little address locality left, making it not very representative.
# Therefore, we just use modulo mapping instead. This results in some data that originally have different addresses
# but use the same cache set to now have the same address. However, this only happens on the modulo boundaries, so 
# hopefully, it doesn't matter much.

def print_asm_line(asm, write_file):
    write_file.write("asm volatile (\n")
    write_file.write(asm)
    write_file.write(f": : : \"{data_reg1}\", \"{data_reg2}\", \"{addr_reg}\");")

for file in trace_files:
    access_count = sum(1 for _ in open(file))
    first_access = random.randint(0, access_count-window_size)
    with open(file, 'r') as accesses_file:
        with open(file + ".c", 'w') as write_file:
            skip_count = 0
            write_count = 0
            
            sequence_base_idx = 0
            sequence_step = 0
            
            
            for line in accesses_file:
                if skip_count < first_access:
                    skip_count += 1
                    continue
                elif write_count == window_size:
                    break
                
                write_count += 1
                
                split = line.strip().split(',')
                address = int(split[0])
                is_read = split[1].strip()=="r"
                distance = int(split[2])
                
                if (address % 4) != 0:
                    print(f"Unaligned address: {address}")
                    exit()
                if split[1].strip()!="r" and split[1].strip()!="w":
                    print(f"Invalid read/write flag: {split[1]}")
                    exit()
                if distance < 0:
                    print(f"Invalid distance: {distance}")
                    exit()
                    
                address_idx = address / 4
                mapped_idx = address_idx % target_idx_range
                mapped_addr = int(mapped_idx * 4) + target_min_addr
                
                if mapped_addr < target_min_addr or mapped_addr > target_max_addr:
                    print(f"Invalid mapped address: {mapped_addr}")
                    exit()
                
                #print(f"Address: {address}, Mapped: {mapped_addr}, isRead: {is_read}, distance: {distance}")    
                
                addr_reg = "r9"
                data_reg1 = "r10"
                data_reg2 = "r11"
                
                if mapped_idx == (sequence_base_idx+sequence_step+1) and sequence_step<127: # Indexed load/store can only use 7 bit immediates
                    # This access sequences the previous
                    sequence_step += 1
                    addr_cal =f"${addr_reg} + {sequence_step}"
                    addr_asm="" # No need to load a new address
                    has_loaded=0
                else:
                    addr_asm = f"\"li ${addr_reg} = {mapped_addr};\"" 
                    sequence_step = 0
                    sequence_base_idx = mapped_idx
                    has_loaded=1
                    addr_cal =f"${addr_reg}"
                
                
                access_asm = f"\"swc [{addr_cal}] = ${data_reg1};\""
                if is_read:
                    access_asm = f"\"lwc ${data_reg2} = [{addr_cal}];\""
                                
                for i in range(distance-has_loaded):
                    print_asm_line(f"\"nop;\"\n", write_file)
                print_asm_line(f"{addr_asm}\n{access_asm}\n", write_file)
            
  
        
        