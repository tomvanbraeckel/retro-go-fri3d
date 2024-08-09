file="$1"
head -c 1048576 "$file" > part1.img
tail -c +1048577 "$file" > part2_start_offset_1048576.img
