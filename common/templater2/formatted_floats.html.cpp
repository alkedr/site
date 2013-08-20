#include <common/templater2/begin_template.hpp>
auto var = "";
auto a = 
TEMPLATE_LAMBDA_BEGIN( )
<html>
<body>
<h1>Hello!</h1>
<h2>This is some text</h2>
<h2>This is some variable: $(var)</h2>
<h3>Numbers:</h3>
<ul>
<h3>MORE NUMBERS</h3>
<ul>
</body>
</html>
TEMPLATE_LAMBDA_END()
