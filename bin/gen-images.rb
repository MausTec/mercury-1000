#!/usr/bin/env ruby
require 'fileutils'

CWD = File.expand_path(File.join(File.dirname(__FILE__), ".."))
Dir.chdir(CWD)

OUT = File.join(CWD, "include", "images")
IN  = File.join(CWD, "res", "images")

# Remove Old
Dir[File.join(OUT, "*.h")].each do |file|
    puts "rm #{file}"
    FileUtils.rm file
end

Dir[File.join(IN, "*.bmp")].each do |file|
    puts "mogrify -format xbm #{file}"
    `mogrify -format xbm #{file}`
    
    basename = File.basename(file, '.bmp')
    xbm_file = File.join(IN, basename + '.xbm')
    xbm = File.read(xbm_file)

    File.write(File.join(OUT, basename + '.h'), <<EOM)
/** 
 * This file was automatically generated using gen-images.rb
 * File: res/#{basename}.bmp
 */
#include "graphics.hpp"

#{xbm.gsub(/char/, 'graphics_image_data_t')}

static graphics_image_t #{basename.upcase} {
    .width = #{basename}_width,
    .height = #{basename}_height,
    .bitmap = #{basename}_bits
};
EOM
    
    FileUtils.rm xbm_file
end