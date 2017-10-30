#!/usr/bin/ruby -w

require 'open3'

AVR_OBJDUMP = '/usr/bin/avr-objdump'
AVR_OBJCOPY = '/usr/bin/avr-objcopy'

if (ARGV.size() != 3)
  puts "usage: #{$0} <elf-file(in)> <bin-file(out)> <xref-file(out)>"
  exit 1
end

elfFileName = ARGV[0]
binFileName = ARGV[1]
xrefFileName = ARGV[2]

system(AVR_OBJCOPY, '-I', 'elf32-avr', '-O', 'binary', elfFileName, binFileName)

File.open(xrefFileName, 'w') do |xref|
  Open3.popen2("#{AVR_OBJDUMP} -t -C #{elfFileName}") do |i, o, t|
    while (line = o.gets)
      if (line =~ /([0-9a-fA-F]+)\s+([lgu!][w ][C ][W ][Ii ][dD ][FfO ])\s+([-+:*.a-zA-Z0-9]+)\s+([0-9a-fA-F]+)\s+([-_:*.a-zA-Z0-9]+)/)
        addr = ($1.to_i(16) / 2).to_s(16)
        type = case $2[6]
               when 'F', 'f'
                 'c'
               else
                 'j'
               end
        section = $3
        label = $5
        if (section == '.text')
          xref.puts type + ' 0x' + addr + ' ' + label
        end
      end
    end
  end
end
