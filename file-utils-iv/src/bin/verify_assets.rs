// src/bin/verify_assets.rs

use std::fs;
use std::path::Path;

fn main() {
    println!("🔍 Mayan Calendar Asset Verification");
    println!("====================================");
    
    // Show current working directory
    let cwd = std::env::current_dir().unwrap();
    println!("📂 Current directory: {}\n", cwd.display());
    
    let mut all_good = true;
    let mut missing_files = Vec::new();
    let mut found_files = Vec::new();
    
    // Check font
    println!("📝 Checking font:");
    let font_path = "assets/fonts/NotoSansMayanNumerals-Regular.ttf";
    if Path::new(font_path).exists() {
        println!("  ✅ {} exists", font_path);
        found_files.push(font_path.to_string());
    } else {
        println!("  ❌ {} MISSING", font_path);
        println!("     Download from: https://github.com/notofonts/mayan-numerals");
        missing_files.push(font_path.to_string());
        all_good = false;
    }
    
    // Check Tzolkin glyphs
    println!("\n🌅 Checking Tzolk'in glyphs:");
    let tzolkin_names = vec![
        ("imix", "Imix"),
        ("akbal", "Ak'b'al"),
        ("kan", "K'an"),
        ("chikchan", "Chikchan"),
        ("kimi", "Kimi"),
        ("manik", "Manik'"),
        ("lamat", "Lamat"),
        ("muluk", "Muluk"),
        ("ok", "Ok"),
        ("chuwen", "Chuwen"),
        ("eb", "Eb'"),
        ("ben", "B'en"),
        ("ix", "Ix"),
        ("men", "Men"),
        ("kib", "Kib'"),
        ("kaban", "Kab'an"),
        ("etznab", "Etz'nab'"),
        ("kawak", "Kawak"),
        ("ajaw", "Ajaw"),
    ];
    
    for (filename, display_name) in tzolkin_names {
        let path = format!("assets/tzolkin/glyphs/{}.png", filename);
        if Path::new(&path).exists() {
            println!("  ✅ {} ({}) exists", filename, display_name);
            found_files.push(path);
        } else {
            println!("  ❌ {} ({}) MISSING", filename, display_name);
            missing_files.push(path);
            all_good = false;
        }
    }
    
    // Check Haab glyphs
    println!("\n📅 Checking Haab' glyphs:");
    let haab_names = vec![
        ("pop", "Pop"),
        ("wo", "Wo'"),
        ("sip", "Sip"),
        ("sotz", "Sotz'"),
        ("sek", "Sek"),
        ("xul", "Xul"),
        ("yaxkin", "Yaxk'in"),
        ("mol", "Mol"),
        ("che", "Ch'en"),  // Note: config has "che" but display is "Ch'en"
        ("yax", "Yax"),
        ("sak", "Sak"),
        ("keh", "Keh'"),
        ("mak", "Mak"),
        ("kankin", "K'ank'in"),
        ("muwan", "Muwan"),
        ("pax", "Pax"),
        ("kayeb", "K'ayeb"),
        ("kumkuk", "Kumk'u"),  // Note: might be "kumku" in some sources
        ("wayeb", "Wayeb'"),
    ];
    
    for (filename, display_name) in haab_names {
        let path = format!("assets/haab/glyphs/{}.png", filename);
        if Path::new(&path).exists() {
            println!("  ✅ {} ({}) exists", filename, display_name);
            found_files.push(path);
        } else {
            println!("  ❌ {} ({}) MISSING", filename, display_name);
            missing_files.push(path);
            all_good = false;
        }
    }
    
    // Create directories if needed
    println!("\n📁 Checking directories:");
    let dirs = vec![
        "assets",
        "assets/fonts",
        "assets/tzolkin",
        "assets/tzolkin/glyphs",
        "assets/haab",
        "assets/haab/glyphs",
    ];
    
    for dir in dirs {
        if Path::new(dir).exists() {
            println!("  ✅ {} exists", dir);
        } else {
            println!("  ⚠️  {} doesn't exist - creating it...", dir);
            fs::create_dir_all(dir).expect("Failed to create directory");
            println!("  ✅ {} created", dir);
        }
    }
    
    // Summary
    println!("\n📊 Summary:");
    println!("===========");
    println!("✅ Found {} files", found_files.len());
    println!("❌ Missing {} files", missing_files.len());
    
    if all_good {
        println!("\n🎉 All assets are present! You're ready to run the calendar!");
    } else {
        println!("\n📋 Missing files list:");
        for file in &missing_files {
            println!("   - {}", file);
        }
        
        println!("\n💡 Tips:");
        println!("1. You'll need to obtain or create the glyph PNG files (128x128 pixels recommended)");
        println!("2. The font can be downloaded from: https://github.com/notofonts/mayan-numerals");
        println!("3. I can see you have glyphs in assets/tzolkin/glyphs - make sure filenames match!");
        println!("4. Make sure filenames match exactly (case-sensitive on Linux/Mac)");
        println!("\nFrom your screenshot, I see you have these Tzolk'in glyphs:");
        println!("ajaw, akbal, ben, chikchan, chuwen, eb, etznab, ik, imix, ix, kaban, kan, kawak, kib");
        println!("\nMake sure they match what the config expects!");
        
        // Create placeholder image code
        println!("\n🎨 Creating placeholder images for missing glyphs...");
        create_placeholder_images(&missing_files);
    }
}

fn create_placeholder_images(missing_files: &[String]) {
    use image::{ImageBuffer, Rgb};
    
    for file in missing_files {
        if file.ends_with(".png") {
            // Create a simple 128x128 placeholder image
            let img = ImageBuffer::from_fn(128, 128, |x, y| {
                // Create a simple pattern
                if (x + y) % 20 < 10 {
                    Rgb([200u8, 200u8, 200u8])
                } else {
                    Rgb([150u8, 150u8, 150u8])
                }
            });
            
            // Extract the glyph name from the path
            let name = file.split('/').last().unwrap_or("unknown")
                .replace(".png", "");
            
            // Save the placeholder
            match img.save(file) {
                Ok(_) => println!("  📝 Created placeholder for: {}", name),
                Err(e) => println!("  ⚠️  Failed to create placeholder for {}: {}", name, e),
            }
        }
    }
    
    println!("\n✅ Placeholder images created! The app should now run, but you'll want to replace these with real glyphs.");
}