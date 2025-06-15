use chrono::{NaiveDate};
use lazy_static::lazy_static;
use std::collections::HashMap;

// First, let's define our astronomical constants
lazy_static! {
    static ref ASTRONOMICAL_CYCLES: HashMap<&'static str, f64> = {
        let mut m = HashMap::new();
        // Basic cycles
        m.insert("synodic_month", 29.530588); // Average length of lunar month
        m.insert("venus_synodic", 583.92);    // Venus synodic period
        m.insert("solar_year", 365.242189);   // Tropical year length
        m.insert("eclipse_year", 346.62);     // Time between similar eclipse conditions
        
        // Maya-specific cycles
        m.insert("tzolkin_cycle", 260.0);     // Length of Tzolkin cycle
        m.insert("haab_cycle", 365.0);        // Length of Haab cycle
        m.insert("calendar_round", 18980.0);  // Least common multiple of Tzolkin and Haab
        m.insert("long_count_cycle", 1872000.0); // Length of Long Count cycle (13 baktuns)
        m
    };

    // Define the solstices and equinoxes for the current epoch
    static ref SEASONAL_DATES: [(i32, i32, &'static str); 4] = [
        (3, 20, "Spring Equinox"),    // Around March 20
        (6, 21, "Summer Solstice"),   // Around June 21
        (9, 22, "Autumn Equinox"),    // Around September 22
        (12, 21, "Winter Solstice"),  // Around December 21
    ];
}

/// Calculates the moon phase for a given Julian Day Number
pub fn moon_phase(jdn: i32) -> String {
    // The lunar synodic month is approximately 29.53059 days
    let lunar_month = ASTRONOMICAL_CYCLES["synodic_month"];
    
    // Calculate the phase angle (0 to 1, where 0 = new moon, 0.5 = full moon)
    // The offset 2451550.1 is the Julian Day for a known new moon (January 6, 2000)
    let phase = ((jdn as f64 - 2451550.1) % lunar_month) / lunar_month;
    
    // Convert the phase to a descriptive string with appropriate emoji
    match phase {
        p if p < 0.0625 => "🌑 New Moon",
        p if p < 0.1875 => "🌒 Waxing Crescent",
        p if p < 0.3125 => "🌓 First Quarter",
        p if p < 0.4375 => "🌔 Waxing Gibbous",
        p if p < 0.5625 => "🌕 Full Moon",
        p if p < 0.6875 => "🌖 Waning Gibbous",
        p if p < 0.8125 => "🌗 Last Quarter",
        p if p < 0.9375 => "🌘 Waning Crescent",
        _ => "🌑 New Moon",
    }.to_string()
}

/// Calculates the Venus phase for a given Julian Day Number
pub fn venus_phase(jdn: i32) -> String {
    // Venus has a synodic period of approximately 583.92 days
    let venus_period = ASTRONOMICAL_CYCLES["venus_synodic"];
    
    // Calculate phase angle (0 to 1)
    // The offset 2451996.706 corresponds to an inferior conjunction of Venus
    let phase = ((jdn as f64 - 2451996.706) % venus_period) / venus_period;
    
    // Venus phases have special significance in Maya astronomy
    match phase {
        p if p < 0.05 => "⭐ Inferior Conjunction",
        p if p < 0.25 => "🌅 Morning Star (Rising)",
        p if p < 0.45 => "⭐ Greatest Western Elongation",
        p if p < 0.55 => "🌄 Morning Star (Setting)",
        p if p < 0.95 => "🌇 Evening Star",
        _ => "⭐ Superior Conjunction",
    }.to_string()
}

/// Determines the Year Bearer (year god) for a given Julian Day Number
pub fn year_bearer(jdn: i32) -> String {
    // The Year Bearer system uses four day signs: Ik', Manik', Eb', and Kab'an
    let year_bearers = [
        "Ik' (White)",
        "Manik' (Deer)",
        "Eb' (Grass)",
        "Kab'an (Earth)",
    ];
    
    // Calculate the year position in the cycle
    let year_position = ((jdn - 2456282).rem_euclid(1461)) / 365;    // 1461 = 4 * 365.25 (approx)
    year_bearers[year_position as usize].to_string()
}

/// Calculates the next seasonal event (solstice or equinox) and days until it
pub fn next_solstice_or_equinox(year: i32, month: i32, day: i32) -> (String, i32) {
    let current_date = NaiveDate::from_ymd_opt(year, month as u32, day as u32).unwrap();
    
    // Find the next seasonal event
    for &(event_month, event_day, event_name) in SEASONAL_DATES.iter() {
        let event_date = NaiveDate::from_ymd_opt(
            if event_month < month { year + 1 } else { year },
            event_month as u32,
            event_day as u32
        ).unwrap();
        
        if event_date > current_date {
            let days_until = event_date.signed_duration_since(current_date).num_days();
            return (event_name.to_string(), days_until as i32);
        }
    }
    
    // If we're past the winter solstice, return next year's spring equinox
    let next_spring = NaiveDate::from_ymd_opt(year + 1, 3, 20).unwrap();
    let days_until = next_spring.signed_duration_since(current_date).num_days();
    ("Spring Equinox".to_string(), days_until as i32)
}

/// Predicts potential eclipse conditions based on the Julian Day Number
pub fn next_eclipse(jdn: i32) -> String {
    // The Saros cycle (223 synodic months) is approximately 6585.32 days
    let saros = ASTRONOMICAL_CYCLES["synodic_month"] * 223.0;
    
    // Calculate position in eclipse cycle
    // The offset 2451550.1 is a known eclipse date
    let eclipse_phase = ((jdn as f64 - 2451550.1) % saros) / saros;
    
    match eclipse_phase {
        p if p < 0.01 => "🌑 Possible Solar Eclipse".to_string(),
        p if (p - 0.5).abs() < 0.01 => "🌕 Possible Lunar Eclipse".to_string(),
        p if p < 0.5 => format!("☀️ {} days until next lunar eclipse", 
            ((0.5 - p) * saros).round() as i32),
        _ => format!("🌙 {} days until next solar eclipse",
            ((1.0 - eclipse_phase) * saros).round() as i32),
    }
}

/// Retrieves historical event for a given Julian Day Number
pub fn historical_event(jdn: i32) -> Option<&'static str> {
    // Access our static HashMap of historical events
    lazy_static! {
        static ref HISTORICAL_EVENTS: HashMap<i32, &'static str> = {
            let mut m = HashMap::new();
            m.insert(584283, "🌎 The Maya creation date (0.0.0.0.0)");
            m.insert(1710534, "📜 Earliest Long Count Date Found (7.16.3.2.13)");
            m.insert(1722559, "🏛️ Dedication of Temple of the Cross at Palenque");
            m.insert(1729974, "⚔️ Teotihuacan Influence Over Tikal Begins");
            m.insert(1738923, "👑 Birth of K'inich Janaab' Pakal I");
            m.insert(1747545, "🗿 Dedication of Temple of Inscriptions at Palenque");
            m
        };
    }
    
    HISTORICAL_EVENTS.get(&jdn).copied()
}