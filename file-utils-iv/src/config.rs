use std::collections::HashMap;

pub const TZOLKIN_GLYPH_PATH: &str = "assets/tzolkin/glyphs/";
pub const HAAB_GLYPH_PATH: &str = "assets/haab/glyphs/";

pub struct Config {
    pub tzolkin_glyphs: HashMap<String, String>,
    pub haab_glyphs: HashMap<String, String>,
}

impl Default for Config {
    fn default() -> Self {
        let mut tzolkin_glyphs = HashMap::new();
        let mut haab_glyphs = HashMap::new();

        // Tzolk'in day glyphs with traditional Maya spellings
        tzolkin_glyphs.insert("imix".to_string(), "assets/tzolkin/glyphs/imix.png".to_string());
        tzolkin_glyphs.insert("ak'b'al".to_string(), "assets/tzolkin/glyphs/akbal.png".to_string());
        tzolkin_glyphs.insert("kan".to_string(), "assets/tzolkin/glyphs/kan.png".to_string());
        tzolkin_glyphs.insert("chikchan".to_string(), "assets/tzolkin/glyphs/chikchan.png".to_string());
        tzolkin_glyphs.insert("kimi".to_string(), "assets/tzolkin/glyphs/kimi.png".to_string());
        tzolkin_glyphs.insert("manik'".to_string(), "assets/tzolkin/glyphs/manik.png".to_string());
        tzolkin_glyphs.insert("lamat".to_string(), "assets/tzolkin/glyphs/lamat.png".to_string());
        tzolkin_glyphs.insert("muluk".to_string(), "assets/tzolkin/glyphs/muluk.png".to_string());
        tzolkin_glyphs.insert("ok".to_string(), "assets/tzolkin/glyphs/ok.png".to_string());
        tzolkin_glyphs.insert("chuwen".to_string(), "assets/tzolkin/glyphs/chuwen.png".to_string());
        tzolkin_glyphs.insert("eb'".to_string(), "assets/tzolkin/glyphs/eb.png".to_string());
        tzolkin_glyphs.insert("ben'".to_string(), "assets/tzolkin/glyphs/ben.png".to_string());
        tzolkin_glyphs.insert("ix".to_string(), "assets/tzolkin/glyphs/ix.png".to_string());
        tzolkin_glyphs.insert("men".to_string(), "assets/tzolkin/glyphs/men.png".to_string());
        tzolkin_glyphs.insert("kib".to_string(), "assets/tzolkin/glyphs/kib.png".to_string());
        tzolkin_glyphs.insert("kaban".to_string(), "assets/tzolkin/glyphs/kaban.png".to_string());
        tzolkin_glyphs.insert("etznab".to_string(), "assets/tzolkin/glyphs/etznab.png".to_string());
        tzolkin_glyphs.insert("kawa".to_string(), "assets/tzolkin/glyphs/kawak.png".to_string());
        tzolkin_glyphs.insert("ajaw".to_string(), "assets/tzolkin/glyphs/ajaw.png".to_string());

        // Haab' month glyphs with traditional Maya spellings
        haab_glyphs.insert("pop".to_string(), "assets/haab/glyphs/pop.png".to_string());
        haab_glyphs.insert("wo".to_string(), "assets/haab/glyphs/wo.png".to_string());
        haab_glyphs.insert("sip".to_string(), "assets/haab/glyphs/sip.png".to_string());
        haab_glyphs.insert("sotz'".to_string(), "assets/haab/glyphs/sotz.png".to_string());
        haab_glyphs.insert("sek".to_string(), "assets/haab/glyphs/sek.png".to_string());
        haab_glyphs.insert("xul".to_string(), "assets/haab/glyphs/xul.png".to_string());
        haab_glyphs.insert("yaxk'in".to_string(), "assets/haab/glyphs/yaxkin.png".to_string());
        haab_glyphs.insert("mol".to_string(), "assets/haab/glyphs/mol.png".to_string());
        haab_glyphs.insert("ch'en".to_string(), "assets/haab/glyphs/che.png".to_string());
        haab_glyphs.insert("yax".to_string(), "assets/haab/glyphs/yax.png".to_string());
        haab_glyphs.insert("sak".to_string(), "assets/haab/glyphs/sak.png".to_string());
        haab_glyphs.insert("keh'".to_string(), "assets/haab/glyphs/keh.png".to_string());
        haab_glyphs.insert("mak".to_string(), "assets/haab/glyphs/mak.png".to_string());
        haab_glyphs.insert("k'ank'in".to_string(), "assets/haab/glyphs/kankin.png".to_string());
        haab_glyphs.insert("muwan".to_string(), "assets/haab/glyphs/muwan.png".to_string());
        haab_glyphs.insert("pax".to_string(), "assets/haab/glyphs/pax.png".to_string());
        haab_glyphs.insert("k'ayeb".to_string(), "assets/haab/glyphs/kayeb.png".to_string());
        haab_glyphs.insert("kumk'u".to_string(), "assets/haab/glyphs/kumkuk.png".to_string());
        haab_glyphs.insert("wayeb".to_string(), "assets/haab/glyphs/wayeb.png".to_string());

        Self {
            tzolkin_glyphs,
            haab_glyphs,
        }
    }
}

impl Config {
    pub fn new() -> Self {
        // This is essentially the same as the default implementation
        Self::default()
    }
}