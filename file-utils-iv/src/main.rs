use std::sync::Arc;
use std::sync::RwLock;
use std::sync::atomic::{AtomicU64, Ordering};
use std::collections::HashMap;
use std::num::NonZeroUsize;
use lru::LruCache;
use chrono::{NaiveDate, NaiveDateTime, Datelike};

use eframe::{App, NativeOptions};
use egui::{self, Context, TextureHandle, ColorImage, TextureOptions, Vec2, ViewportBuilder};
use tracing::{error, info, Level};
use tracing_subscriber::EnvFilter;

// Local module imports
mod config;
mod date_utils;
mod astronomical;

use config::Config;
use date_utils::{gregorian_to_jdn, tzolkin_date, haab_date, TzolkinDate, HaabDate};
use astronomical::{
    moon_phase,
    venus_phase,
    year_bearer,
    next_solstice_or_equinox,
    next_eclipse,
    historical_event,
};

// Enum for Glyph Types
#[derive(Debug, Clone, Copy)]
pub enum GlyphType {
    Tzolkin,
    Haab,
}

// Performance Metrics
#[derive(Default)]
pub struct Metrics {
    calculation_time: AtomicU64,
    glyph_load_time: AtomicU64,
    render_time: AtomicU64,
    cache_hits: AtomicU64,
    cache_misses: AtomicU64,
}

impl Metrics {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn record_calculation(&self, duration: std::time::Duration) {
        self.calculation_time.fetch_add(duration.as_micros() as u64, Ordering::Relaxed);
    }

    pub fn record_cache_hit(&self) {
        self.cache_hits.fetch_add(1, Ordering::Relaxed);
    }

    pub fn record_cache_miss(&self) {
        self.cache_misses.fetch_add(1, Ordering::Relaxed);
    }

    pub fn report(&self) -> String {
        format!(
            "Performance Metrics:\n\
             Calculation Time: {}µs\n\
             Cache Hits: {}\n\
             Cache Misses: {}\n\
             Cache Hit Rate: {:.2}%",
            self.calculation_time.load(Ordering::Relaxed),
            self.cache_hits.load(Ordering::Relaxed),
            self.cache_misses.load(Ordering::Relaxed),
            self.cache_hit_rate() * 100.0
        )
    }

    fn cache_hit_rate(&self) -> f64 {
        let hits = self.cache_hits.load(Ordering::Relaxed) as f64;
        let misses = self.cache_misses.load(Ordering::Relaxed) as f64;
        let total = hits + misses;
        if total > 0.0 {
            hits / total
        } else {
            0.0
        }
    }
}

// Texture Cache
pub struct TextureCache {
    tzolkin_textures: HashMap<String, TextureHandle>,
    haab_textures: HashMap<String, TextureHandle>,
}

// Calendar Cache
pub struct CalendarCache {
    cache: LruCache<i32, CalendarData>,
}

impl CalendarCache {
    pub fn new(capacity: NonZeroUsize) -> Self {
        Self {
            cache: LruCache::new(capacity),
        }
    }
    
    pub fn get_calendar_data(&mut self, days: i32) -> Option<CalendarData> {
        self.cache.get(&days).cloned()
    }
    
    pub fn put_calendar_data(&mut self, days: i32, data: CalendarData) {
        self.cache.put(days, data);
    }
}

// Glyph Error Handling
#[derive(Debug, thiserror::Error)]
pub enum GlyphError {
    #[error("Failed to open file: {0}")]
    FileError(std::io::Error),
    #[error("Memory mapping failed: {0}")]
    MmapError(std::io::Error),
    #[error("Failed to load image: {0}")]
    ImageLoadError(#[from] image::ImageError),
    #[error("Invalid glyph dimensions: {0}x{1}, expected 128x128")]
    InvalidDimensions(u32, u32),
}

#[derive(Debug, Clone, Copy, Hash, Eq, PartialEq)]
pub struct LongCount {
    baktun: i32,
    katun: i32,
    tun: i32,
    uinal: i32,
    kin: i32,
}

impl LongCount {
    pub fn from_days(days: i32) -> Self {
        let baktun = days / 144_000;
        let rem1 = days % 144_000;
        let katun = rem1 / 7_200;
        let rem2 = rem1 % 7_200;
        let tun = rem2 / 360;
        let rem3 = rem2 % 360;
        let uinal = rem3 / 20;
        let kin = rem3 % 20;
        Self { baktun, katun, tun, uinal, kin }
    }

    pub fn to_days(&self) -> i32 {
        self.baktun * 144_000 +
        self.katun * 7_200 +
        self.tun * 360 +
        self.uinal * 20 +
        self.kin
    }
}

#[derive(Clone)]
pub struct CalendarData {
    long_count: LongCount,
    tzolkin: TzolkinDate,
    haab: HaabDate,
    moon_phase: String,
    venus_phase: String,
    year_bearer: String,
    next_solstice: (String, i32),
    eclipse_status: String,
    historical_event: Option<String>,
    gregorian_date: NaiveDate,
    julian_day_number: i32,
    days_since_creation: i32,
}

impl CalendarData {
    pub fn new(date: NaiveDateTime) -> Self {
        // Get the current date components
        let year = date.year();
        let month = date.month() as i32;
        let day = date.day() as i32;
        
        // Calculate Julian Day Number using the function from date_utils
        let jdn = gregorian_to_jdn(year, month, day);
        
        // Mayan epoch: August 11, 3114 BCE = JDN 584283
        let mayan_epoch_jdn = 584283;
        let days_since_creation = jdn - mayan_epoch_jdn;
        
        info!("Date: {}-{}-{}, JDN: {}, Days since creation: {}", 
              year, month, day, jdn, days_since_creation);
        
        let long_count = LongCount::from_days(days_since_creation);
        let tzolkin = tzolkin_date(days_since_creation);
        let haab = haab_date(days_since_creation);
        
        // Calculate astronomical data
        let moon = moon_phase(jdn);
        let venus = venus_phase(jdn);
        let bearer = year_bearer(jdn);
        let eclipse = next_eclipse(jdn);
        let (solstice_name, days_to_solstice) = next_solstice_or_equinox(year, month, day);
        let historical = historical_event(jdn);
        
        Self {
            long_count,
            tzolkin,
            haab,
            moon_phase: moon,
            venus_phase: venus,
            year_bearer: bearer,
            next_solstice: (solstice_name, days_to_solstice),
            eclipse_status: eclipse,
            historical_event: historical.map(|s| s.to_string()),
            gregorian_date: date.date(),
            julian_day_number: jdn,
            days_since_creation,
        }
    }
}

fn to_mayan_numeral_string(long_count: &LongCount) -> String {
    format!("{}.{}.{}.{}.{}", 
        to_mayan_digit(long_count.baktun),
        to_mayan_digit(long_count.katun),
        to_mayan_digit(long_count.tun),
        to_mayan_digit(long_count.uinal),
        to_mayan_digit(long_count.kin))
}

fn to_mayan_digit(n: i32) -> String {
    // Define the Unicode code points for Mayan numerals (0-19)
    let base_codepoint = 0x1D2E0;  // Starting code point for Mayan numerals
    let codepoint = base_codepoint + (n as u32);
    
    match char::from_u32(codepoint) {
        Some(c) => {
            info!("Generated Mayan numeral for {}: U+{:X} = '{}'", n, codepoint, c);
            c.to_string()
        },
        None => {
            error!("Failed to create Mayan numeral for {}", n);
            n.to_string() // Fallback to regular number
        }
    }
}

pub struct GlyphRenderer {
    cache: Arc<RwLock<TextureCache>>,
    config: Config,
    metrics: Arc<Metrics>,
    ctx: Context,
}

impl GlyphRenderer {
    pub fn new(ctx: &Context, config: Config) -> Self {
        Self {
            cache: Arc::new(RwLock::new(TextureCache {
                tzolkin_textures: HashMap::new(),
                haab_textures: HashMap::new(),
            })),
            config,
            metrics: Arc::new(Metrics::new()),
            ctx: ctx.clone(),
        }
    }

    pub fn get_texture(&self, glyph_type: GlyphType, name: &str) -> Option<TextureHandle> {
        // Normalize the name to match config keys
        let normalized_name = name.to_lowercase();
        
        info!("Looking for glyph: {} (normalized: {})", name, normalized_name);
        
        // Get the path from the configuration
        let path = match glyph_type {
            GlyphType::Tzolkin => self.config.tzolkin_glyphs.get(&normalized_name),
            GlyphType::Haab => self.config.haab_glyphs.get(&normalized_name),
        };

        let path = match path {
            Some(p) => p,
            None => {
                error!("No path found for glyph: {} (type: {:?})", normalized_name, glyph_type);
                return None;
            }
        };

        // Check the cache
        let mut cache = self.cache.write().unwrap();
        let cached_texture = match glyph_type {
            GlyphType::Tzolkin => cache.tzolkin_textures.get(path).cloned(),
            GlyphType::Haab => cache.haab_textures.get(path).cloned(),
        };

        if let Some(texture) = cached_texture {
            self.metrics.record_cache_hit();
            return Some(texture);
        }

        self.metrics.record_cache_miss();

        // Load image
        let start_time = std::time::Instant::now();
        let image = match image::open(path) {
            Ok(img) => {
                info!("Successfully loaded glyph image: {}", path);
                img
            }
            Err(e) => {
                error!("Failed to load image at {}: {}", path, e);
                return None;
            }
        };

        let size = [image.width() as usize, image.height() as usize];
        let image_buffer = image.to_rgba8();
        let pixels = image_buffer.as_flat_samples();
        let image_data = ColorImage::from_rgba_unmultiplied(size, pixels.as_slice());

        // Load texture into egui
        let texture = self.ctx.load_texture(
            &format!("{}_{}", glyph_type as u8, normalized_name), 
            image_data, 
            TextureOptions::default()
        );

        // Cache it
        match glyph_type {
            GlyphType::Tzolkin => { 
                cache.tzolkin_textures.insert(path.clone(), texture.clone()); 
            },
            GlyphType::Haab => { 
                cache.haab_textures.insert(path.clone(), texture.clone()); 
            },
        }

        let load_time = start_time.elapsed();
        info!("Loaded glyph {} in {:?}", path, load_time);

        Some(texture)
    }
}

pub struct MayanCalendar {
    current_time: chrono::DateTime<chrono::Local>,
    calendar_data: CalendarData,
    last_calendar_update: chrono::NaiveDateTime,
    cache: Arc<RwLock<CalendarCache>>,
    glyph_renderer: GlyphRenderer,
    metrics: Arc<Metrics>,
}

impl MayanCalendar {
    pub fn new(ctx: &Context) -> Result<Self, Box<dyn std::error::Error>> {
        let metrics = Arc::new(Metrics::new());
        let cache = Arc::new(RwLock::new(CalendarCache::new(NonZeroUsize::new(100).unwrap())));
        let glyph_renderer = GlyphRenderer::new(ctx, Config::default());
        let now = chrono::Local::now().naive_local();

        Ok(Self {
            current_time: chrono::Local::now(),
            calendar_data: CalendarData::new(now),
            last_calendar_update: now,
            cache: Arc::clone(&cache),
            glyph_renderer,
            metrics,
        })
    }

    pub fn update_calendar_data(&mut self) {
        let now = chrono::Local::now();
        if now != self.current_time {
            let start = std::time::Instant::now();
            self.current_time = now;
            self.calendar_data = CalendarData::new(self.current_time.naive_local());
            self.metrics.record_calculation(start.elapsed());
            
            info!(
                "Updated calendar: Long Count {}.{}.{}.{}.{}, Tzolkin {} {}, Haab {} {}",
                self.calendar_data.long_count.baktun,
                self.calendar_data.long_count.katun,
                self.calendar_data.long_count.tun,
                self.calendar_data.long_count.uinal,
                self.calendar_data.long_count.kin,
                self.calendar_data.tzolkin.number,
                self.calendar_data.tzolkin.yucatec_name,
                self.calendar_data.haab.day,
                self.calendar_data.haab.yucatec_month
            );
        }
    }

    pub fn render(&mut self, ctx: &Context) {
        let desired_size = Vec2::new(128.0, 128.0);
        
        egui::CentralPanel::default().show(ctx, |ui| {
            // Title and Clock
            ui.vertical_centered(|ui| {
                ui.heading("🌎 Mayan Calendar 🌎");
                ui.label(
                    egui::RichText::new(format!("{}", self.current_time.format("%Y-%m-%d %H:%M:%S")))
                        .size(20.0)
                        .strong()
                );
            });
            
            ui.separator();
            
            // Long Count Display
            ui.group(|ui| {
                ui.label(egui::RichText::new("Long Count").size(18.0).strong());
                
                // Numeric display
                ui.label(format!(
                    "{}.{}.{}.{}.{}",
                    self.calendar_data.long_count.baktun,
                    self.calendar_data.long_count.katun,
                    self.calendar_data.long_count.tun,
                    self.calendar_data.long_count.uinal,
                    self.calendar_data.long_count.kin
                ));
                
                // Mayan numerals
                let mayan_text = to_mayan_numeral_string(&self.calendar_data.long_count);
                ui.label(
                    egui::RichText::new(format!("Mayan: {}", mayan_text))
                        .family(egui::FontFamily::Name("mayan".into()))
                        .size(32.0)
                );
            });
            
            ui.separator();
            
            // Tzolkin and Haab displays side by side
            ui.horizontal(|ui| {
                // Tzolkin
                ui.group(|ui| {
                    ui.vertical(|ui| {
                        ui.label(egui::RichText::new("Tzolk'in").size(16.0).strong());
                        ui.label(format!(
                            "{} {}",
                            self.calendar_data.tzolkin.number,
                            self.calendar_data.tzolkin.yucatec_name
                        ));
                        
                        if let Some(tzolkin_glyph) = self.glyph_renderer.get_texture(
                            GlyphType::Tzolkin,
                            &self.calendar_data.tzolkin.yucatec_name,
                        ) {
                            ui.add(egui::Image::new(&tzolkin_glyph).fit_to_exact_size(desired_size));
                        } else {
                            ui.colored_label(
                                egui::Color32::RED, 
                                format!("Missing glyph: {}", self.calendar_data.tzolkin.yucatec_name)
                            );
                        }
                    });
                });
                
                // Haab
                ui.group(|ui| {
                    ui.vertical(|ui| {
                        ui.label(egui::RichText::new("Haab'").size(16.0).strong());
                        ui.label(format!(
                            "{} {}",
                            self.calendar_data.haab.day,
                            self.calendar_data.haab.yucatec_month
                        ));
                        
                        if let Some(haab_glyph) = self.glyph_renderer.get_texture(
                            GlyphType::Haab,
                            &self.calendar_data.haab.yucatec_month,
                        ) {
                            ui.add(egui::Image::new(&haab_glyph).fit_to_exact_size(desired_size));
                        } else {
                            ui.colored_label(
                                egui::Color32::RED, 
                                format!("Missing glyph: {}", self.calendar_data.haab.yucatec_month)
                            );
                        }
                    });
                });
            });
            
            ui.separator();
            
            // Astronomical Information
            ui.group(|ui| {
                ui.label(egui::RichText::new("Astronomical Information").size(16.0).strong());
                ui.label(format!("Moon Phase: {}", self.calendar_data.moon_phase));
                ui.label(format!("Venus Phase: {}", self.calendar_data.venus_phase));
                ui.label(format!("Year Bearer: {}", self.calendar_data.year_bearer));
                ui.label(format!("Eclipse Status: {}", self.calendar_data.eclipse_status));
                ui.label(format!(
                    "Next {}: {} days",
                    self.calendar_data.next_solstice.0,
                    self.calendar_data.next_solstice.1
                ));
            });
            
            // Historical Event (if any)
            if let Some(event) = &self.calendar_data.historical_event {
                ui.separator();
                ui.group(|ui| {
                    ui.label(egui::RichText::new("Historical Event").size(16.0).strong());
                    ui.label(event);
                });
            }
            
            // Debug Information
            ui.separator();
            ui.collapsing("Debug Information", |ui| {
                ui.label(format!("JDN: {}", self.calendar_data.julian_day_number));
                ui.label(format!("Days since creation: {}", self.calendar_data.days_since_creation));
                ui.label(self.metrics.report());
            });
        });
    }
}

impl App for MayanCalendar {
    fn update(&mut self, ctx: &Context, _frame: &mut eframe::Frame) {
        if (chrono::Local::now() - self.current_time).num_seconds() >= 1 {
            self.update_calendar_data();
        }
        self.render(ctx);
        ctx.request_repaint_after(std::time::Duration::from_secs(1));
    }
}

fn configure_fonts(ctx: &Context) -> Result<(), Box<dyn std::error::Error>> {
    let mut fonts = egui::FontDefinitions::default();
    
    // Try to load the Mayan numerals font
    match std::fs::read("assets/fonts/NotoSansMayanNumerals-Regular.ttf") {
        Ok(font_data) => {
            info!("Font file loaded successfully, size: {} bytes", font_data.len());
            
            fonts.font_data.insert(
                "mayan_numerals".to_owned(),
                egui::FontData::from_owned(font_data)
            );
            
            // Register for all font families
            fonts.families.get_mut(&egui::FontFamily::Proportional)
                .unwrap()
                .insert(0, "mayan_numerals".to_owned());
            
            fonts.families.get_mut(&egui::FontFamily::Monospace)
                .unwrap()
                .insert(0, "mayan_numerals".to_owned());
            
            // Create dedicated Mayan family
            fonts.families.insert(
                egui::FontFamily::Name("mayan".into()),
                vec!["mayan_numerals".to_owned()]
            );
            
            ctx.set_fonts(fonts);
            info!("Font configuration completed successfully");
        }
        Err(e) => {
            error!("Failed to load Mayan numerals font: {}. Continuing without it.", e);
            // Continue without the font - numbers will display as regular digits
        }
    }
    
    Ok(())
}

fn main() -> Result<(), eframe::Error> {
    // Initialize logging
    tracing_subscriber::FmtSubscriber::builder()
        .with_env_filter(
            EnvFilter::from_default_env()
                .add_directive(Level::INFO.into())
                .add_directive("mayan_calendar=debug".parse().unwrap())
        )
        .with_thread_ids(true)
        .with_thread_names(true)
        .with_file(true)
        .with_line_number(true)
        .with_target(false)
        .compact()
        .init();
    
    info!("Starting Mayan Calendar application");
    
    // Set up application options
    let options = NativeOptions {
        viewport: ViewportBuilder::default()
            .with_inner_size([900.0, 700.0])
            .with_title("Mayan Calendar"),
        vsync: true,
        ..Default::default()
    };
    
    // Run the application
    eframe::run_native(
        "Mayan Calendar",
        options,
        Box::new(|cc| {
            // Configure fonts before creating the app
            if let Err(e) = configure_fonts(&cc.egui_ctx) {
                error!("Font configuration error: {}", e);
            }
            
            match MayanCalendar::new(&cc.egui_ctx) {
                Ok(app) => Box::new(app),
                Err(e) => {
                    error!("Failed to create app: {}", e);
                    panic!("Application initialization failed: {}", e);
                }
            }
        }),
    )
}