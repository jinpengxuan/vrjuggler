<BLOCKQUOTE>
  {include ../common/member_prototype.html.inc}

  <BLOCKQUOTE>
    {private false}
      {if descr}
	<P>
	  {include ../common/descr.html.inc}
	</P>
      {endif}
      {else}
	{if overrides}
	  <P>
	    {include ../common/descr.html.inc}
	  </P>
	{endif}
      {endelse}

      {include ../common/contract.html.inc}
    {endprivate}

    {if ARGS}
      <P>
	{ARGS}
      </P>
    {endif}
    {if RETURNS}
      <P>
	{RETURNS}
      </P>
    {endif}
  </BLOCKQUOTE>
</BLOCKQUOTE>