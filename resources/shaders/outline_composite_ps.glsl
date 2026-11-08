SW_PIXEL_INPUT vec2 PixelTexCoord;
SW_PIXEL_OUTPUT

uniform sampler2D Texture;
uniform sampler2D VisibleTexture;
uniform mediump vec4 OutlineParams[5]; // Pixel size / outline radius / hidden opacity, selected color, focused color, highlight color, hover color.

SW_SHADER_PRECISION vec4 OutlineMaskCoverage(SW_SHADER_PRECISION vec2 Position)
{
	if (Position.x < 0.0 || Position.y < 0.0 || Position.x > 1.0 || Position.y > 1.0)
		return vec4(0.0);

	return texture2D(Texture, Position);
}

void main()
{
	SW_SHADER_PRECISION vec4 Coverage = OutlineMaskCoverage(PixelTexCoord);
	SW_SHADER_PRECISION vec4 Expanded = Coverage;
	SW_SHADER_PRECISION vec3 ExpandedOpacity = vec3(Coverage.rb, 0.0);
	SW_SHADER_PRECISION vec2 PixelSize = OutlineParams[0].xy;
	SW_SHADER_PRECISION float Radius = min(OutlineParams[0].z, 8.0);

	// A disk of physical pixels keeps thin silhouettes connected on HiDPI
	// displays. Constant loop bounds also work with desktop GLSL 1.10.
	for (int Y = -8; Y <= 8; Y++)
	{
		if (abs(float(Y)) > Radius)
			continue;

		for (int X = -8; X <= 8; X++)
		{
			SW_SHADER_PRECISION vec2 Offset = vec2(float(X), float(Y));

			if (dot(Offset, Offset) <= Radius * Radius)
			{
				SW_SHADER_PRECISION vec4 SampleCoverage = OutlineMaskCoverage(PixelTexCoord + PixelSize * Offset);
				SW_SHADER_PRECISION float Opacity = 1.0 - smoothstep(0.0, Radius, max(length(Offset) - 1.0, 0.0));

				Expanded = max(Expanded, SampleCoverage);

				// the hover line runs inside its silhouette
				ExpandedOpacity = max(ExpandedOpacity, vec3(SampleCoverage.rb, 1.0 - SampleCoverage.a) * Opacity);
			}
		}
	}

	SW_SHADER_PRECISION float Border = max(Expanded.r - Coverage.r, 0.0);
	SW_SHADER_PRECISION float BorderOpacity = max(ExpandedOpacity.x - Coverage.r, 0.0);
	SW_SHADER_PRECISION float HighlightOpacity = max(ExpandedOpacity.y - Coverage.b, 0.0) * OutlineParams[3].a;
	SW_SHADER_PRECISION float HoverOpacity = Coverage.a * ExpandedOpacity.z * OutlineParams[4].a;
	SW_SHADER_PRECISION float Visible = texture2D(VisibleTexture, PixelTexCoord).r;
	SW_SHADER_PRECISION float Hidden = Coverage.r * (1.0 - Visible);
	SW_SHADER_PRECISION vec4 FillColor = mix(OutlineParams[1], OutlineParams[2], Coverage.g);
	SW_SHADER_PRECISION vec4 OutlineColor = mix(OutlineParams[1], OutlineParams[2], Expanded.g);
	SW_SHADER_PRECISION vec3 SelectionColor = mix(FillColor.rgb, OutlineColor.rgb, Border);
	SW_SHADER_PRECISION float SelectionOpacity = max(Hidden * OutlineParams[0].w, BorderOpacity);
	SW_SHADER_PRECISION float BelowOpacity = SelectionOpacity + HighlightOpacity * (1.0 - SelectionOpacity);
	SW_SHADER_PRECISION vec3 BelowColor = SelectionColor * SelectionOpacity + OutlineParams[3].rgb * HighlightOpacity * (1.0 - SelectionOpacity);

	// the hover line stays on top
	SW_SHADER_PRECISION float Opacity = HoverOpacity + BelowOpacity * (1.0 - HoverOpacity);
	SW_SHADER_PRECISION vec3 Color = OutlineParams[4].rgb * HoverOpacity + BelowColor * (1.0 - HoverOpacity);

	gl_FragColor = vec4(Color / max(Opacity, 0.0001), Opacity);
}
