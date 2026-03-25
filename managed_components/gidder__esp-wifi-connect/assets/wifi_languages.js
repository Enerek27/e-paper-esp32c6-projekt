const default_lang = 'en-US'
// You can remove whole langage or add new language here:
const translations = {
    'cz-CZ': {
        language_name: 'Čeština',
        title: 'Konfigurace Wi-Fi',
        saved_wifi: 'Uložená Wi-Fi',
        new_wifi: 'Nová Wi-Fi',
        password: 'Heslo:',
        connect: 'Připojit',
        select_wifi: 'Vyberte 2,4G WiFi z níže uvedeného seznamu:',
        connection_failed: 'Připojení se nezdařilo',
        unable_connect: 'Nelze se připojit k Wi-Fi',
        invalid_ssid: 'Neplatné SSID',
        will_restart: 'Zařízení se restartuje za',
        seconds: 'sekundy',
    },
    'en-US': {
        language_name: 'English',
        title: 'WiFi Configuration',
        saved_wifi: 'Saved WiFi',
        new_wifi: 'New WiFi',
        password: 'Password:',
        connect: 'Connect',
        select_wifi: 'Select an 2.4G WiFi from the list below:',
        connection_failed: 'Connection failed',
        unable_connect: 'Unable to connect to WiFi',
        invalid_ssid: 'Invalid SSID',
        will_restart: 'Device will restart in',
        seconds: 'seconds',
    },
    'ja-JP': {
        language_name: '日本語',
        title: 'ネットワーク設定',
        saved_wifi: '保存済みのWi-Fi',
        new_wifi: '新しいWi-Fi',
        password: 'パスワード:',
        connect: '接続',
        select_wifi: '以下のリストから2.4G Wi-Fiを選択してください:',
        connection_failed: '接続に失敗しました',
        unable_connect: 'WiFiに接続できません',
        invalid_ssid: '無効なSSID',
        will_restart: 'デバイスは',
        seconds: '秒後に再起動します',
    },
    'ko-KR': {
        language_name: '한국어',
        title: '네트워크 설정',
        saved_wifi: '저장된 Wi-Fi',
        new_wifi: '새로운 Wi-Fi',
        password: '비밀번호:',
        connect: '연결',
        select_wifi: '아래 목록에서 2.4G Wi-Fi를 선택하세요:',
        connection_failed: '연결에 실패했습니다',
        unable_connect: 'WiFi에 연결할 수 없습니다',
        invalid_ssid: '잘못된 SSID',
        will_restart: '장치가',
        seconds: '초 후 재시작됩니다',
    },
    'sk-SK': {
        language_name: 'Slovenčina',
        title: 'Konfigurácia Wi-Fi',
        saved_wifi: 'Uložené Wi-Fi',
        new_wifi: 'Nová Wi-Fi',
        password: 'Heslo:',
        connect: 'Pripojiť',
        select_wifi: 'Vyberte si 2,4G WiFi z nižšie uvedeného zoznamu:',
        connection_failed: 'Pripojenie zlyhalo',
        unable_connect: 'Nepodarilo sa pripojiť k Wi-Fi',
        invalid_ssid: 'Neplatné SSID',
        will_restart: 'Zariadenie sa reštartuje o',
        seconds: 'sekundy',
    },
    'zh-CN': {
        language_name: '中文',
        title: 'WiFi 配置',
        saved_wifi: '已保存的 WiFi',
        new_wifi: '新的 WiFi',
        password: '密码:',
        connect: '连接',
        select_wifi: '从下面列表选择 2.4G WiFi:',
        connection_failed: '连接失败',
        unable_connect: '无法连接到 WiFi',
        invalid_ssid: '无效的 SSID',
        will_restart: '设备将在',
        seconds: '秒后重启',
    },
    'zh-TW': {
        language_name: '繁體中文',
        title: '網路設定',
        saved_wifi: '已儲存的 Wi-Fi',
        new_wifi: '新的 Wi-Fi',
        password: '密碼:',
        connect: '連接',
        select_wifi: '從下方列表選擇 2.4G Wi-Fi:',
        connection_failed: '連線失敗',
        unable_connect: '無法連接到 WiFi',
        invalid_ssid: '無效的 SSID',
        will_restart: '裝置將在',
        seconds: '秒後重新啟動',
    },
};

var languages_select = {}
var shorts = {}
// Add short translation names ('en', ..)
for (const [id, dict] of Object.entries(translations)) {
    languages_select[id] = dict.language_name;
    const parts = id.split("-");
    let short_id;
    for(var i = 1; i < parts.length; i++) {
        short_id = parts.slice(0,1).join("-");
        if (!translations.hasOwnProperty(short_id)) {
            shorts[short_id] = id;
        }
    }
}
for (const [short_id, id] of Object.entries(shorts)) {
    translations[short_id] = translations[id];
}
shorts = null;
