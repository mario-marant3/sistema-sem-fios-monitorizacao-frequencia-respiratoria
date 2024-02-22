<!DOCTYPE html>
<html lang="pt-PT">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Calendário Leituras</title>
    <link href="https://cdn.jsdelivr.net/npm/fullcalendar@5.10.1/main.min.css" rel="stylesheet">
    <style>
        #calendar {
            height: 800px;
        }
    </style>
</head>
<body>
    <div id="calendar"></div>
    <script src="https://cdn.jsdelivr.net/npm/moment@2.29.1/moment.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/jquery@3.6.0/dist/jquery.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/fullcalendar@5.10.1/main.min.js"></script>
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            var calendarEl = document.getElementById('calendar');

            var calendar = new FullCalendar.Calendar(calendarEl, {
                initialView: 'dayGridMonth',
                events: "eventos.php",
                eventClick: function(info) {
                    var formattedTime = moment(info.event.start).format('YYYY-MM-DD HH:mm');
                    alert('Evento: ' + info.event.title + '\nData e Hora: ' + formattedTime);

                    var eventId = info.event.id;
                    window.location.href = "grafico.php?id=" + eventId;
                }
            });
            calendar.render();
        });
    </script>
</body>
</html>
