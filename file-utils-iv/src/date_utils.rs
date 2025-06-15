pub fn gregorian_to_jdn(year: i32, month: i32, day: i32) -> i32 {
// Convert a Gregorian date to Julian Day Number (JDN)
  let a = (14 - month) / 12;
  let y = year + 4800 - a;
  let m = month + 12 * a - 3;
  day + ((153 * m + 2) / 5) + 365 * y + y / 4 - y / 100 + y / 400 - 32045
}

#[derive(Clone)]
pub struct TzolkinDate {
    pub number: i32,
    pub yucatec_name: String,
}

impl TzolkinDate {
    pub fn new(number: i32, name: &str) -> Self {
        Self {
            number,
            yucatec_name: name.to_string(),
        }
    }
}

pub fn tzolkin_date(days: i32) -> TzolkinDate {
    let number = (((days + 3) % 13 + 13) % 13) + 1;
    let yucatec_names = [
        "Imix", "Ik'", "Ak'b'al", "K'an", "Chikchan",
        "Kimi", "Manik'", "Lamat", "Muluk", "Ok",
        "Chuwen", "Eb'", "B'en", "Ix", "Men",
        "Kib'", "Kab'an", "Etz'nab'", "Kawak", "Ajaw"
    ];
    let index = (((days + 19) % 20 + 20) % 20) as usize;
    TzolkinDate {
        number,
        yucatec_name: yucatec_names[index].to_string(),
    }
}

#[derive(Clone)]
pub struct HaabDate {
    pub day: i32,
    pub yucatec_month: String,
}

impl HaabDate {
    pub fn new(day: i32, month: &str) -> Self {
        Self {
            day,
            yucatec_month: month.to_string(),
        }
    }
}

pub fn haab_date(days: i32) -> HaabDate {
    let haab_day = ((days + 348) % 365 + 365) % 365;
    let month_index = haab_day / 20;
    let day = haab_day % 20;
    
    let yucatec_months = [
        "Pop", "Wo'", "Sip", "Sotz'", "Sek", "Xul", "Yaxkin", "Mol",
        "Ch'en", "Yax", "Zac", "Ceh", "Mac", "Kankin", "Muan", "Pax",
        "Kayab", "Kumk'u", "Wayeb'"
    ];
    
    HaabDate {
        day,
        yucatec_month: yucatec_months[month_index as usize].to_string(),
    }
}